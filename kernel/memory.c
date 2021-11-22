#include "memory.h"
#include "stdint.h"
#include "print.h"
#include "bitmap.h"
#include "global.h"
#include "debug.h"
#include "stdint.h"
#include "string.h"

#define PG_SIZE 4096
#define MEM_BITMAP_BASE 0xc009a000 // related to PCB
#define K_HEAP_START 0xc0100000 // 3GB + 1MB

#define PDE_IDX(addr)  ( (addr & 0xffc00000) >> 22 ) // front  10 bits
#define PTE_IDX(addr)  ( (addr & 0x003ff000) >> 12 ) // middle 10 bits

struct pool {
    struct bitmap pool_bitmap;
    uint32_t phy_addr_start;
    uint32_t pool_size;
};

struct pool kernel_pool, user_pool;
struct virtual_addr kernel_vaddr; // alloc virtual address for kernel

static void mem_pool_init(uint32_t all_mem) {
    put_str("  mem_pool_init start\n");
    uint32_t page_table_size = PG_SIZE * 256;

    // memory
    uint32_t used_mem = page_table_size + 0x100000;
    uint32_t free_mem = all_mem - used_mem;
    // pages
    uint16_t all_free_pages = free_mem / PG_SIZE;
    uint16_t kernel_free_pages = all_free_pages / 2;
    uint16_t user_free_pages   = all_free_pages - kernel_free_pages;
    // bitmap length
    uint32_t kbm_length = kernel_free_pages / 8;
    uint32_t ubm_length = user_free_pages / 8;
    // physical address start
    uint32_t kp_start = used_mem;
    uint32_t up_start = kp_start + kernel_free_pages * PG_SIZE;
    // init pool
    kernel_pool.phy_addr_start = kp_start;
    kernel_pool.pool_size = kernel_free_pages * PG_SIZE;
    kernel_pool.pool_bitmap.btmp_bytes_len = kbm_length;
    kernel_pool.pool_bitmap.bits = (void*)MEM_BITMAP_BASE;

    user_pool.phy_addr_start  = up_start;
    user_pool.pool_size = user_free_pages *PG_SIZE;
    user_pool.pool_bitmap.btmp_bytes_len = ubm_length;
    user_pool.pool_bitmap.bits = (void*)(MEM_BITMAP_BASE + kbm_length);

    // print memory pool info
    put_str("    kernel_pool_bitmap_start: ");
    put_int((int)kernel_pool.pool_bitmap.bits);
    put_str("    kernel_pool_phy_addr_start: ");
    put_int(kernel_pool.phy_addr_start);
    put_str("\n");
    put_str("    user_pool_bitmap_start: ");
    put_int((int)user_pool.pool_bitmap.bits);
    put_str("    user_pool_phy_addr_start: ");
    put_int(user_pool.phy_addr_start);
    put_str("\n");

    // init bitmap for kernel & user pool
    bitmap_init(&(kernel_pool.pool_bitmap));
    bitmap_init(&(user_pool.pool_bitmap));

    // init virtual address pool
    kernel_vaddr.vaddr_start = K_HEAP_START;
    kernel_vaddr.vaddr_bitmap.bits = (void*)(MEM_BITMAP_BASE + kbm_length + ubm_length);
    kernel_vaddr.vaddr_bitmap.btmp_bytes_len = kbm_length;
    bitmap_init(&(kernel_vaddr.vaddr_bitmap));

    put_str("  mem_pool_init is done\n");
}

void mem_init() {
    put_str("mem init start\n");
    uint32_t mem_bytes_total = *((uint32_t*)(0xb00));
    mem_pool_init(mem_bytes_total);
    put_str("mem_init_done\n");
}

uint32_t* pte_ptr(uint32_t vaddr) {
    return (uint32_t*)(0xffc00000 + ((vaddr & 0xffc00000) >> 10) + PTE_IDX(vaddr)*4);
}

uint32_t* pde_ptr(uint32_t vaddr) {
    return (uint32_t*)(0xfffff000 + PDE_IDX(vaddr)*4);
}

// alloc some virtual address
static void* va_alloc(enum pool_flag pf, uint32_t pg_cnt) {
    int vaddr_start, bit_idx_start;
    uint32_t cnt = 0;

    if(pf == PF_KERNEL) {
        bit_idx_start = bitmap_alloc(&kernel_vaddr.vaddr_bitmap, pg_cnt);
        if(bit_idx_start == -1) return NULL;
        while (cnt < pg_cnt) {
            bitmap_set(&kernel_vaddr.vaddr_bitmap, bit_idx_start + cnt, 1);
            cnt++;
        }
        vaddr_start = kernel_vaddr.vaddr_start + bit_idx_start * PG_SIZE;
    }

    else {
        // user memory pool
        // todo: when need to make user process working
    }

    return (void*)vaddr_start;
}

// alloc one page physical address
static void* pa_alloc(struct pool* m_pool) {
    int bit_idx = bitmap_alloc(&m_pool->pool_bitmap, 1);
    if(bit_idx == -1) return NULL;
    bitmap_set(&m_pool->pool_bitmap, bit_idx, 1);
    return (void*) (m_pool->phy_addr_start + (bit_idx*PG_SIZE));
}

static void page_table_set(void* va, void* pa) {
    uint32_t vaddr = (uint32_t)va;
    uint32_t paddr = (uint32_t)pa;
    uint32_t* pde  = pde_ptr(vaddr);
    uint32_t* pte  = pte_ptr(vaddr);

    // pde doesn't exist
    if (*pde & 0x00000000) {
        uint32_t pde_phyaddr = (uint32_t)pa_alloc(&kernel_pool);
        *pde = (pde_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
        memset((void*)((int)pte & 0xfffff000), 0 , PG_SIZE);
    }

    // pte exists
    if(*pte & 0x00000001)  PANIC("pte repeat");
    *pte = (paddr | PG_US_U | PG_RW_W | PG_P_1);
}

void* malloc_page(enum pool_flag pf, uint32_t pg_cnt) {
    ASSERT(pg_cnt > 0 && pg_cnt < 3840);
    void* vaddr_start = va_alloc(pf, pg_cnt);
    if(vaddr_start == NULL) return NULL;
    uint32_t vaddr = (uint32_t)vaddr_start;
    struct pool* m_pool = pf & PF_KERNEL ? &kernel_pool : &user_pool;
    for(uint32_t cnt=0; cnt < pg_cnt; cnt++) {
        void* paddr = pa_alloc(m_pool);
        if(paddr == NULL) {
            // todo: when failed, roll back all mapped pages
            return NULL;
        }
        page_table_set((void*)vaddr, paddr);
        vaddr += PG_SIZE;
    }
    return vaddr_start;
}

void* get_kernel_pages(uint32_t pg_cnt) {
    void* vaddr = malloc_page(PF_KERNEL, pg_cnt);
    if(vaddr != NULL) memset(vaddr, 0, pg_cnt*PG_SIZE);
    return vaddr;
}