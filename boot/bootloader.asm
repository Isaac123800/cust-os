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

    ; Load 38 sectors starting at sector 2
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

    ; Enable A20
    in al, 0x92
    or al, 00000010b
    out 0x92, al

    ; Load GDT
    lgdt [gdt_descriptor]

    ; Enter protected mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; Far jump into 32-bit code
    jmp 0x08:protected_mode


print:
    lodsb

    cmp al, 0
    je .done

    mov ah, 0x0E
    int 0x10

    jmp print

.done:
    ret


disk_error:
    mov si, error_message
    call print

.hang:
    cli
    hlt
    jmp .hang


; --------------------------------
; Global Descriptor Table
; --------------------------------

gdt_start:

gdt_null:
    dq 0

gdt_code:
    dw 0xFFFF
    dw 0
    db 0
    db 10011010b
    db 11001111b
    db 0

gdt_data:
    dw 0xFFFF
    dw 0
    db 0
    db 10010010b
    db 11001111b
    db 0

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start


; --------------------------------
; Protected mode
; --------------------------------

bits 32

protected_mode:

    ; Set data segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Kernel stack
    mov esp, 0x90000

    ; Copy kernel:
    ; 0x8000 -> 0x100000

    mov esi, 0x8000
    mov edi, 0x100000

    ; 19036 / 4 = 4759 DWORDs
    mov ecx, 4759

    cld
    rep movsd

    ; Jump to kernel entry
    jmp 0x100000


boot_drive db 0

loading_message db "Loading kernel...", 13, 10, 0
loaded_message  db "Kernel loaded!", 13, 10, 0
error_message   db "Disk read error!", 13, 10, 0


times 510 - ($ - $$) db 0
dw 0xAA55



