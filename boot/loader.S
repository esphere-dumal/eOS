%include "boot.inc"
section loader vstart=LOADER_BASE_ADDR
LOADER_STACK_TOP equ LOADER_BASE_ADDR

jmp loader_start

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

gdt_ptr dw GDT_LIMIT
        dd GDT_BASE

loadermsg db '2 loader in real.'

loader_start:
    ; print some info out
    mov sp, LOADER_BASE_ADDR
    mov bp, loadermsg
    mov cx, 17
    mov ax, 0x1301 ; ah = 13, al = 01
    mov bx, 0x001f ; bh = 00 -> page is 0, bl = 1f -> blue backend with pink frontend
    mov dx, 0x1800 ; dh, dl -> x, y
    int 0x10

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

[bits 32]
p_mode_start:
    mov ax, SELECTOR_DATA
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, LOADER_STACK_TOP
    mov ax, SELECTOR_VIDEO
    mov gs, ax

    mov byte [gs:160], 'P'
    mov byte [gs:161], 0x1F

    jmp $