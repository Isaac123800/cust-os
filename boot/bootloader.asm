bits 16
org 0x7C00

start:
    cli

    mov [boot_drive], dl

    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    mov si, loading_message
    call print

    ; --------------------------------------------------------
    ; Load the kernel
    ;
    ; Kernel size:
    ; 23036 bytes
    ;
    ; Required sectors:
    ; 45 × 512 = 23040 bytes
    ;
    ; BIOS sector 2 = LBA 1
    ;
    ; Load temporarily at 0x8000.
    ; --------------------------------------------------------

    mov ah, 0x02
    mov al, 45
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, [boot_drive]

    mov bx, 0x8000

    int 0x13
    jc disk_error

    mov si, loaded_message
    call print


    ; --------------------------------------------------------
    ; Enable A20
    ; --------------------------------------------------------

    in al, 0x92
    or al, 00000010b
    and al, 11111110b
    out 0x92, al


    ; --------------------------------------------------------
    ; Load GDT
    ; --------------------------------------------------------

    lgdt [gdt_descriptor]


    ; --------------------------------------------------------
    ; Enter protected mode
    ; --------------------------------------------------------

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp 0x08:protected_mode


; ============================================================
; REAL MODE PRINT
; ============================================================

print:
    lodsb

    cmp al, 0
    je .done

    mov ah, 0x0E
    int 0x10

    jmp print

.done:
    ret


; ============================================================
; DISK ERROR
; ============================================================

disk_error:
    mov si, error_message
    call print

.hang:
    cli
    hlt
    jmp .hang


; ============================================================
; PROTECTED MODE
; ============================================================

bits 32

protected_mode:

    ; --------------------------------------------------------
    ; Set up flat 32-bit segments
    ; --------------------------------------------------------

    mov ax, 0x10

    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov esp, 0x90000

    cld


    ; --------------------------------------------------------
    ; Copy kernel
    ;
    ; 45 sectors × 512 bytes = 23040 bytes
    ;
    ; 23040 / 4 = 5760 DWORDs
    ;
    ; Source:
    ;   0x8000
    ;
    ; Destination:
    ;   0x100000
    ; --------------------------------------------------------

    mov esi, 0x8000
    mov edi, 0x100000
    mov ecx, 5760

    rep movsd


    ; --------------------------------------------------------
    ; Jump to kernel
    ;
    ; linker.ld places the kernel at 1 MiB.
    ; --------------------------------------------------------

    jmp 0x100000


; ============================================================
; GDT
; ============================================================

align 8

gdt_start:

    ; Null descriptor
    dq 0x0000000000000000

    ; 32-bit code segment
    dq 0x00CF9A000000FFFF

    ; 32-bit data segment
    dq 0x00CF92000000FFFF

gdt_end:


gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start


; ============================================================
; DATA
; ============================================================

boot_drive db 0

loading_message db "Loading kernel...", 13, 10, 0
loaded_message  db "Kernel loaded!", 13, 10, 0
error_message   db "Disk read error!", 13, 10, 0


; ============================================================
; BOOT SIGNATURE
; ============================================================

times 510 - ($ - $$) db 0

dw 0xAA55

