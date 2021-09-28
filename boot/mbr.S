%include "boot.inc"
SECTION MBR vstart=0x7c00
    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov sp, 0x7c00
    mov ax, 0xb800
    mov gs, ax
;---------------------------------------------------
    mov ax, 0x0600
    mov bx, 0x0700
    mov cx, 0
    mov dx, 0x184f
    int 0x10
    mov byte [gs:0x00], '1'
    mov byte [gs:0x01], 0xA4
    mov byte [gs:0x02], ' '
    mov byte [gs:0x03], 0xA4
    mov byte [gs:0x04], 'M'
    mov byte [gs:0x05], 0xA4
    mov byte [gs:0x06], 'B'
    mov byte [gs:0x07], 0xA4
    mov byte [gs:0x08], 'A'
    mov byte [gs:0x09], 0xA4
;---------------------------------------------------
    mov eax, LOADER_START_SECTOR
    mov bx, LOADER_BASE_ADDR
    mov cx, 1
    call rd_disk_m_16

    jmp LOADER_BASE_ADDR
;---------------------------------------------------
rd_disk_m_16:
    mov esi, eax
    mov di, cx
    ; setting the disk to read
    mov dx, 0x01f2
    mov al, cl
    out dx, al

    mov eax, esi
    mov cl, 8
    ; LBA28 3*8 + 4 + 4(11xx)
    ; LBA address 7 ~ 0 in 0x01f3
    mov dx, 0x01f3
    out dx, al
    ; LBA address 15 ~ 8 in 0x01f4
    shr eax, cl
    mov dx, 0x01f4
    out dx, al
    ; LBA address 23 ~16 in 0x01f5
    shr eax, cl
    mov dx, 0x01f5
    out dx, al
    ; LBA decice 1110xxxx in 0x01f6
    shr eax, cl
    and al, 0x0f
    or al, 0xe0
    mov dx, 0x01f6
    out dx, al
    ; command in 0x01f7
    mov dx, 0x01f7
    mov al, 0x20
    out dx, al
  ; checking disk status
  .not_ready:
    nop
    in al, dx
    and al, 0x88
    cmp al, 0x08
    jnz .not_ready
    ; read data
    mov ax, di
    mov dx, 256
    mul dx
    mov cx, ax
    mov dx, 0x01f0
  ; read data
  .go_on_read:
    in ax, dx
    mov [bx], ax    ; bx starts at LOADER_BASE_ADDRESS
    add bx, 2
    loop .go_on_read
    ret

    times 510-($-$$) db 0
    db 0x55, 0xaa