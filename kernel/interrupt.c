#include "interrupt.h"
#include "stdint.h"
#include "global.h"
#include "io.h"

#define IDT_DESC_CNT 0x21
#define PIC_M_CTRL 0x20
#define PIC_M_DATA 0X21
#define PIC_S_CTRL 0Xa0
#define PIC_S_DATA 0xa1

struct gate_desc {
    uint16_t        func_offset_low_word;
    uint16_t        selector;
    uint8_t         dcount;
    uint8_t         attribute;
    uint16_t        func_offset_high_word;
};


static struct gate_desc idt[IDT_DESC_CNT];

extern intr_handler intr_entry_table[IDT_DESC_CNT];

// init 8295A
static void pic_init() {
    outb (PIC_M_CTRL, 0x11);
    outb (PIC_M_DATA, 0X20);
    outb (PIC_M_DATA, 0X04);
    outb (PIC_M_DATA, 0X01);

    outb (PIC_S_CTRL, 0x11);
    outb (PIC_S_DATA, 0x28);
    outb (PIC_S_DATA, 0x02);
    outb (PIC_S_DATA, 0x01);

    // turn on IR0 which is timer
    outb (PIC_M_DATA, 0xfe);
    outb (PIC_S_DATA, 0Xff);

    put_str(  "pic_init is done\n");
}

// create interrupt gate descrition
static void make_idt_desc(struct gate_desc* p_gdesc, uint8_t attr, intr_handler func) {
    p_gdesc->func_offset_low_word = (uint32_t)func & 0x0000FFFF;
    p_gdesc->selector = SELECTOR_K_CODE;
    p_gdesc->dcount = 0;
    p_gdesc->attribute = attr;
    p_gdesc->func_offset_high_word = (uint32_t)func & 0xFFFF0000;
}


// init interrupt description table
static void idt_desc_init() {
    for(int i=0; i<IDT_DESC_CNT; i++) {
        make_idt_desc(&idt[i], IDT_DESC_ATTR_DPL0, intr_entry_table[i]);
    }
    put_str("  idt_desc_init is done\n");
}


// init all interrupt jobs
void idt_init() {
    put_str("idt_init start\n");
    idt_desc_init();
    pic_init();

    // load idt
    uint64_t idt_operand = ((sizeof(idt) - 1) | ((uint64_t)((uint32_t)idt << 16)));
    asm volatile("lidt %0" : : "m" (idt_operand));
    put_str("idt_init is done\n");
}