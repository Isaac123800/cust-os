bits 16
org 0x7C00

start:
    cli

    mov [boot_drive], dl

    ; Set up real-mode segments
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    mov si, loading_message
    call print

    ; Load kernel: 38 sectors starting at BIOS sector 2
    mov ah, 0x02
    mov al, 38
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, [boot_drive]

    ; Load kernel into 0x8000
    mov bx, 0x8000

    int 0x13
    jc disk_error

    mov si, loaded_message
    call print

    ; STOP HERE
    cli
    hlt
    jmp $


disk_error:
    mov si, error_message
    call print

.hang:
    cli
    hlt
    jmp .hang


print:
    lodsb

    cmp al, 0
    je .done

    mov ah, 0x0E
    int 0x10

    jmp print

.done:
    ret


boot_drive db 0

loading_message db "Loading kernel...", 13, 10, 0
loaded_message  db "Kernel loaded!", 13, 10, 0
error_message   db "Disk read error!", 13, 10, 0


times 510 - ($ - $$) db 0
dw 0xAA55

