; loader and the kernel
LOADER_BASE_ADDR    equ 0x900
LOADER_START_SECTOR equ 0x2

; gdt description ----------
DESC_G_4K equ  1_00000000000000000000000b ; high 23 bit: 4kb per segment
DESC_D_32 equ  1_0000000000000000000000b  ; high 22 bit: is 32 mode
DESC_L    equ  0_000000000000000000000b    ; high 21 bit: not 64 mode
DESC_AVL  equ  0_00000000000000000000b     ; high 20 bit: not using this one

; high 19~16 bit: (4gb /4 kb) - 1 = 0xFFFFF, all one
DESC_LIMIT_CODE2 equ 1111_0000000000000000b 
DESC_LIMIT_DATA2 equ DESC_LIMIT_CODE2
DESC_LIMIT_VIDEO2 equ 0000_0000000000000000b

DESC_P equ 1_000000000000000b               ; high 15 bit: present in CPU ? yes

; 4 types of DPL: 4 privilege level
DESC_DPL_0 equ 00_0000000000000b            
DESC_DPL_1 equ 01_0000000000000b
DESC_DPL_2 equ 10_0000000000000b
DESC_DPL_3 equ 11_0000000000000b

; high 12 bit: 0 -> system, 1 -> data & code
DESC_S_CODE equ  1_000000000000b            
DESC_S_DATA equ  DESC_S_CODE
DESC_S_sys equ  0_000000000000b

; high 11 ~ 8 bit: type
DESC_TYPE_CODE equ 1000_00000000b
DESC_TYPE_DATA equ 0010_00000000b

; three diffrent type of segment, limit\s\type  has diffrence
DESC_CODE_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + \
    DESC_L + DESC_AVL + DESC_LIMIT_CODE2 + \
    DESC_P + DESC_DPL_0 + DESC_S_CODE + DESC_TYPE_CODE + 0x00

DESC_DATA_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + \
    DESC_L + DESC_AVL + DESC_LIMIT_DATA2 + \
    DESC_P + DESC_DPL_0 + DESC_S_DATA + DESC_TYPE_DATA + 0x00

DESC_VIDEO_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + \
    DESC_L + DESC_AVL + DESC_LIMIT_VIDEO2 + \
    DESC_P + DESC_DPL_0 + DESC_S_DATA + DESC_TYPE_DATA + 0x0b

; selector ------------
PRL0 equ 00b
PRL1 equ 01b
PRL2 equ 10b
PRL3 equ 11b

TI_GDT equ 000b
TI_LDT equ 100b