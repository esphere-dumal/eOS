%include "boot.inc"
section loader vstart=LOADER_BASE_ADDR

; gdt high part and low part, each is 4 byte
GDT_BASE: dd 0x00000000
          dd 0x00000000

CODE_DESC: dd 0x0000FFFF
           dd DESC_CODE_HIGH4

DATA_STACK_DESC: dd 0x0000FFFF
                 dd DESC_DATA_HIGH4

VIDEO_DESC: dd 0x80000007   ; (0xBFFFF-0xB8000)/4k = 0x7
            dd DESC_VIDEO_HIGH4

GDT_SIZE  equ $ - GDT_BASE
GDT_LIMIT equ GDT_SIZE - 1
times 120 dd 0

SELECTOR_CODE  equ (0x0001 << 3) + TI_GDT + RPL0
SELECTOR_DATA  equ (0x0002 << 3) + TI_GDT + RPL0
SELECTOR_VIDEO equ (0x0003 << 3) + TI_GDT + RPL0

; address here is 0xb00, will be used in kernel
; 0x900 + 0x200 = 0xb00
; 0x200 = 4*8 + 120*4 = 512 = 0xb00
total_mem_bytes dd 0

gdt_ptr dw GDT_LIMIT
        dd GDT_BASE

ards_buf times 244 db 0 
ards_nr  dw 0

loader_start:
    xor ebx, ebx
    mov edx, 0x534d4150
    mov di,  ards_buf

  .e820_mem_get_loop:
    mov eax, 0x0000e820
    mov ecx, 20
    int 0x15
    jc .e820_failed_so_try_e801
    add di, cx
    inc word [ards_nr]
    cmp ebx, 0
    jnz .e820_mem_get_loop


    mov cx, [ards_nr]
    mov ebx, ards_buf
    xor edx, edx
  ; bubble sort to find the highest mem
  .find_max_mem_area:
    mov eax, [ebx]
    add eax, [ebx+8]
    add ebx, 20
    cmp ebx, eax
    jge .next_ards
    mov edx, eax
  .next_ards:
    loop .find_max_mem_area
    jmp  .mem_get_ok


  .e820_failed_so_try_e801:
    mov ax, 0xe801
    int 0x15
    jc  .e801_failed_so_try_88
    ; 1. 15 MB
    mov cx, 0x400
    mul cx
    shl edx, 16
    and eax, 0x0000FFFF
    or  edx, eax
    add edx, 0x100000 ; 1MB memory hole
    mov esi, edx
    ; 2. 4 GB
    xor eax, eax
    mov ax,  bx
    mov ecx, 0x10000
    mul ecx

    add esi, edx
    mov edx, esi
    jmp .mem_get_ok


  .e801_failed_so_try_88:
    mov ah, 0x88
    int 0x15
    jc .error_hlt
    and eax, 0x0000FFFF

    mov cx, 0x400
    mul cx
    shl edx, 16
    or  edx, eax
    add edx, 0x100000

  .mem_get_ok:
    mov [total_mem_bytes], edx

    ; enter protect mode:
    ; 1. turn on A20
    ; 2. load gdt
    ; 3. set cr0's pe bit to 1

    ; turn on A20
    in al, 0x92
    or al, 0000_0010B
    out 0x92, al

    ; load gdt
    lgdt [gdt_ptr]

    ; set cr0's pe bit to 1
    mov eax, cr0
    or  eax, 0x00000001
    mov cr0, eax

    jmp dword SELECTOR_CODE:p_mode_start    ; refresh pipeline

  .error_hlt:
    hlt

[bits 32]
p_mode_start:
    mov ax, SELECTOR_DATA
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, LOADER_STACK_TOP
    mov ax, SELECTOR_VIDEO
    mov gs, ax

; load kernel into buffer
    mov eax, KERNEL_START_SECTOR
    mov ebx, KERNEL_BIN_BASE_ADDR
    mov ecx, 200
    call rd_disk_m_32

    call setup_page
    sgdt [gdt_ptr]
    mov ebx, [gdt_ptr + 2]
    or  dword [ebx + 0x18 + 4], 0xc0000000
    add dword [gdt_ptr + 2], 0xc0000000
    add esp, 0xc0000000
    mov eax, PAGE_DIR_TABLE_POS
    mov cr3, eax

    mov eax, cr0
    or  eax, 0x80000000
    mov cr0, eax

    lgdt [gdt_ptr]

    jmp SELECTOR_CODE:enter_kernel

enter_kernel:
    call kernel_init
    mov esp, 0xc009f000
    jmp KERNEL_ENTRY_POINT

kernel_init:
    xor eax, eax
    xor ebx, ebx ; ebx for address for program header table
    xor ecx, ecx ; cx  for program header nums
    xor edx, edx ; dx for size of program headers

    mov dx,  [KERNEL_BIN_BASE_ADDR + 42]  ; e_phentsize, size of program header
    mov ebx, [KERNEL_BIN_BASE_ADDR + 28]  ; e_phoff 
    add ebx, KERNEL_BIN_BASE_ADDR
    mov cx,  [KERNEL_BIN_BASE_ADDR + 44]  ; e_phnum
  .each_segment:
    cmp byte [ebx + 0], PT_NULL
    je .PTNULL
    push dword [ebx + 16] ; p_filesz -> size
    mov eax, [ebx + 4]
    add eax, KERNEL_BIN_BASE_ADDR
    push eax   ; p_offset + kernel.bin_phiscal_address -> src
    push dword [ebx + 8] ; p_vaddr -> dst
    call mem_cpy
    add esp, 12

  .PTNULL:
    add ebx, edx
    loop .each_segment
    ret

; copy byte by byte
; dst, src, size 3 arguments in stack
mem_cpy:
    cld
    push ebp
    mov  ebp, esp
    push ecx            ; rep will use ecx, so need to back up

    mov edi, [ebp + 8]  ; dst
    mov esi, [ebp + 12] ; src
    mov ecx, [ebp + 16] ; size
    rep movsb

    pop ecx
    pop ebp
    ret


; create pagetable
setup_page:
    mov ecx, 4096
    mov esi, 0
  .clear_page_dir:
    mov byte [PAGE_DIR_TABLE_POS + esi], 0
    inc esi
    loop .clear_page_dir

  .create_pde:
    mov eax, PAGE_DIR_TABLE_POS
    add eax, 0x1000
    mov ebx, eax
    or  eax, PG_US_U | PG_RW_W | PG_P

    mov [PAGE_DIR_TABLE_POS + 0x0], eax
    mov [PAGE_DIR_TABLE_POS + 0xc00], eax ; 0xc00 and above belongs to kernel space

    sub eax, 0x1000
    mov [PAGE_DIR_TABLE_POS + 4092], eax  ; the last entry point to itself

; create pte
    mov ecx, 256    ; 256 * 4kb = 1024 kb = 1mb
    mov esi, 0
    mov edx, PG_US_S | PG_RW_W | PG_P
  .create_pte:
    mov [ebx+esi*4], edx
    add edx, 4096
    inc esi
    loop .create_pte


    mov eax, PAGE_DIR_TABLE_POS
    add eax, 0x2000
    or  eax, PG_US_U | PG_RW_W | PG_P
    mov ebx, PAGE_DIR_TABLE_POS
    mov ecx, 254
    mov esi, 769
  .create_kernel_pde:
    mov [ebx+esi*4], eax
    inc esi
    add eax, 0x1000
    loop .create_kernel_pde
    ret

rd_disk_m_32:	   
    mov esi, eax
    mov di, cx
    ; setting the disk to read
    mov dx, 0x1f2
    mov al, cl
    out dx, al
    
    mov eax, esi
    mov cl, 8
    ; LBA28 3*8 + 4 + 4(11xx)
    ; LBA address 7 ~ 0 in 0x01f3
    mov dx, 0x1f3
    out dx, al                          
    ; LBA address 15 ~ 8 in 0x01f4
    shr eax, cl
    mov dx, 0x1f4
    out dx, al
    ; LBA address 23 ~16 in 0x01f5
    shr eax, cl
    mov dx, 0x1f5
    out dx, al
    ; LBA decice 1110xxxx in 0x01f6
    shr eax, cl
    and al, 0x0f
    or  al, 0xe0
    mov dx, 0x1f6
    ; command in 0x01f7
    out dx, al
    mov dx,0x1f7
    mov al,0x20                        
    out dx,al
    ; checking disk status
    .not_ready:		   
      nop
      in  al, dx
      and al, 0x88
      cmp al, 0x08
      jnz .not_ready
      ; read data
      mov ax, di	  
      mov dx, 256   
      mul dx
      mov cx, ax 
      mov dx, 0x1f0
    .go_on_read:
      in ax, dx
      mov [ebx], ax
      add ebx, 2
      loop .go_on_read
      ret