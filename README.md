# cust-os
CustOS- The operating system where you have full control. You can use this in a vm.

Custos commands are used through the command prompt. The credits command displays the creators of Custos, Isaac Polomski, Roshan Inbasekar and Kirthis Kirubaventhan. The cmdlist command shows every available command and whether it is enabled, disabled, or protected; it cannot be disabled. The echo <text> command prints any text you enter back onto the screen, for example echo Hello displays Hello. The clear command clears the screen. The disable -<command> command disables a command so it cannot be used until it is enabled again, while the enable -<command> command restores a disabled command. The cmdlist, enable, and disable commands are protected and cannot be disabled. If you try to use a command that has been disabled, Custos will display error: Command not Found or not Enabled.


bits 16
org 0x7C00

start:
    cli
    xor ax, ax
    mov ds, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    ; Enter VGA Mode 13h: 320x200, 256 colours
    mov ax, 0x0013
    int 0x10

    ; Set VGA framebuffer segment
    mov ax, 0xA000
    mov es, ax

    ; Fill the screen with blue (colour 1)
    xor di, di
    mov al, 1
    mov cx, 64000
    rep stosb

    ; Rectangle: x=50, y=40, width=80, height=40
    mov word [row], 40
    mov word [rows_left], 40

draw_row:
    ; Calculate offset = y * 320 + 50
    mov ax, [row]
    mov bx, 320
    mul bx
    add ax, 50
    mov di, ax

    ; Draw one horizontal red row (colour 4)
    mov al, 4
    mov cx, 80
    rep stosb

    ; Move to the next row
    inc word [row]
    dec word [rows_left]
    jnz draw_row

hang:
    cli
    hlt
    jmp hang

row       dw 0
rows_left dw 0

times 510 - ($ - $$) db 0
dw 0xAA55
