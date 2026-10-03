bits 16
org 0x7C00

start:
    cli

    mov si, message

print:
    lodsb
    cmp al, 0
    je halt

    mov ah, 0x0E
    int 0x10

    jmp print

halt:
    cli
    hlt
    jmp halt

message db "CustOS bootloader working!", 0

times 510 - ($ - $$) db 0
dw 0xAA55
