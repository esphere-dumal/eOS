T1_GDT equ 0
RPL0   equ 0
SELECTOR_VIDEO equ (0x0003 << 3) + T1_GDT + RPL0

[bits 32]
section .text
; buffer for int 
put_int_buffer dq 0

global put_char
global put_str
global put_int

put_int:
    pushad
    mov ebp, eax
    mov eax, [esp + 36] ; 4 + 8*4
    mov edx, eax
    mov edi, 7
    mov ecx, 8
    mov ebx, put_int_buffer

  .int_16based_4bit:  
    and edx, 0x0000000F
    cmp edx, 9
    jg .is_A2F
    add edx, '0'
    jmp .store

  .is_A2F:
    sub edx, 10
    add edx, 'A'

  .store:
    mov [put_int_buffer + edi], dl
    dec edi
    shr eax, 4
    mov edx, eax
    loop .int_16based_4bit

  .ready_to_print:
    inc edi

  .skip_prefix_0:
    cmp edi, 8
    je .full0

  .go_on_skip:
    mov cl, [put_int_buffer + edi]
    inc edi
    cmp cl, '0'
    je .skip_prefix_0
    dec edi
    jmp .put_each_num

  .full0:
    mov cl, '0'

  .put_each_num:
    push ecx
    call put_char
    add esp, 4
    inc edi
    mov cl, [put_int_buffer + edi]
    cmp edi, 8
    jl .put_each_num
    popad
    ret

put_str:
    push ebx
    push ecx
    xor ecx, ecx
    mov ebx, [esp + 12] ; the str to be printed
  .goon:
    mov cl, [ebx]
    cmp cl, 0
    jz  .str_over
    push ecx
    call put_char
    add esp, 4
    inc ebx
    jmp .goon 
  .str_over:
    pop ecx
    pop ebx
    ret


put_char:
    pushad
    mov ax, SELECTOR_VIDEO
    mov gs, ax

    ; get the cursor postion
    ; firstly high 8 bits, then low 8 bits
    mov dx, 0x03d4
    mov al, 0x0e
    out dx, al
    mov dx, 0x03d5
    in  al, dx
    mov ah, al
    mov dx, 0x03d4
    mov al, 0x0f
    out dx, al
    mov dx, 0x03d5
    in  al, dx
    ; put the cursor postion into bx
    mov bx, ax
    mov ecx, [esp + 36] ; 4*8 + 4 = 36
    ; put a char for diffrent situation
    cmp cl, 0xd
    jz .is_carriage_return
    cmp cl, 0xa
    jz .is_line_feed
    cmp cl, 0x8
    jz .is_backspace
    jmp .put_other

  .is_backspace:
    dec bx
    shl bx, 1
    mov byte [gs:bx], 0x20
    inc bx
    mov byte [gs:bx], 0x07
    shr bx, 1
    jmp .set_cursor

  .put_other:
    shl bx, 1
    mov [gs:bx], cl
    inc bx
    mov byte [gs:bx], 0x07
    shr bx, 1
    inc bx
    cmp bx, 2000
    jl .set_cursor

  .is_line_feed:
  .is_carriage_return:
    xor dx, dx
    mov ax, bx
    mov si, 80
    div si
    sub bx, dx

  ; CR return
  .is_carriage_return_end: 
    add bx, 80
    cmp bx, 2000
  ; LF return, cursor + 80
  .is_line_feed_end:
    jl .set_cursor

  .roll_screen:
    cld
    mov ecx, 960 ; 1920 * 2 / 4 = 960

    mov esi, 0xc00b80a0
    mov edi, 0xc00b8000
    rep movsd

    mov ebx, 3840
    mov ecx, 80

  .cls:
    mov word [gs:ebx], 0x0720
    add ebx, 2
    loop .cls
    mov bx, 1920

  .set_cursor:
    mov dx, 0x03d4
    mov al, 0x0e
    out dx, al
    mov dx, 0x03d5
    mov al, bh
    out dx, al
    mov dx, 0x03d4
    mov al, 0x0f
    out dx, al
    mov dx, 0x03d5
    mov al, bl
    out dx, al
    
  .put_char_done:
    popad
    ret
    
global set_cursor
set_cursor:
   pushad
   mov bx, [esp+36]
   mov dx, 0x03d4
   mov al, 0x0e
   out dx, al
   mov dx, 0x03d5
   mov al, bh
   out dx, al


   mov dx, 0x03d4
   mov al, 0x0f
   out dx, al
   mov dx, 0x03d5 
   mov al, bl
   out dx, al
   popad
   ret