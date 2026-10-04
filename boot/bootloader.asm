bits 16
org 0x7C00

start:
    cli

    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    mov [boot_drive], dl

    ; --------------------------------------------------------
    ; Print loading message
    ; --------------------------------------------------------

    mov si, loading_message
    call print


    ; --------------------------------------------------------
    ; Check BIOS Extended Disk Services
    ; --------------------------------------------------------

    mov ah, 0x41
    mov bx, 0x55AA
    mov dl, [boot_drive]

    int 0x13

    jc disk_error

    cmp bx, 0xAA55
    jne disk_error

    test cx, 1
    jz disk_error


    ; --------------------------------------------------------
    ; Set up Disk Address Packet
    ;
    ; Read:
    ;     45 sectors
    ;
    ; Starting at:
    ;     LBA 1
    ;
    ; Destination:
    ;     0000:8000
    ; --------------------------------------------------------

    mov word [dap_count], 45

    mov word [dap_offset], 0x8000
    mov word [dap_segment], 0x0000

    mov dword [dap_lba_low], 1
    mov dword [dap_lba_high], 0


    ; --------------------------------------------------------
    ; Read using INT 13h Extensions
    ; AH = 42
    ; --------------------------------------------------------

    mov si, dap

    mov ah, 0x42
    mov dl, [boot_drive]

    int 0x13

    jc disk_error


    ; --------------------------------------------------------
    ; Kernel loaded
    ; --------------------------------------------------------

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


    ; --------------------------------------------------------
    ; 32-bit far jump
    ; --------------------------------------------------------

    db 0x66
    db 0xEA

    dd protected_mode
    dw 0x08


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
    ; Set flat data segments
    ; --------------------------------------------------------

    mov ax, 0x10

    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax


    ; --------------------------------------------------------
    ; Kernel stack
    ; --------------------------------------------------------

    mov esp, 0x90000

    cld


    ; --------------------------------------------------------
    ; Copy kernel
    ;
    ; 45 sectors
    ; 45 × 512 = 23040 bytes
    ;
    ; 23040 / 4 = 5760 DWORDS
    ;
    ; 0x8000 → 0x100000
    ; --------------------------------------------------------

    mov esi, 0x8000
    mov edi, 0x100000

    mov ecx, 5760

    rep movsd


    ; --------------------------------------------------------
    ; Jump to kernel
    ; --------------------------------------------------------

    mov eax, 0x100000

    jmp eax


; ============================================================
; GDT
; ============================================================

align 8

gdt_start:

    ; Null
    dq 0x0000000000000000

    ; 32-bit code
    dq 0x00CF9A000000FFFF

    ; 32-bit data
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
; DISK ADDRESS PACKET
; ============================================================

align 4

dap:
    db 0x10
    db 0

dap_count:
    dw 45

dap_offset:
    dw 0x8000

dap_segment:
    dw 0

dap_lba_low:
    dd 1

dap_lba_high:
    dd 0


; ============================================================
; BOOT SIGNATURE
; ============================================================

times 510 - ($ - $$) db 0

dw 0xAA55

