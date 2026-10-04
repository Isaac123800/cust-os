bits 16
org 0x7C00

start:
    cli

    ; --------------------------------------------------------
    ; Set up real-mode segments first
    ; --------------------------------------------------------

    xor ax, ax

    mov ds, ax
    mov es, ax
    mov ss, ax

    mov sp, 0x7C00

    mov [boot_drive], dl

    mov si, loading_message
    call print

    ; --------------------------------------------------------
    ; Load kernel
    ;
    ; 45 sectors
    ; BIOS sector 2 = LBA 1
    ; 45 × 512 = 23040 bytes
    ;
    ; Temporary address: 0x8000
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
    ; REAL MODE MARKER A
    ; Disk read succeeded.
    ; --------------------------------------------------------

    call mark_A

    ; --------------------------------------------------------
    ; Enable A20
    ; --------------------------------------------------------

    in al, 0x92

    or al, 00000010b
    and al, 11111110b

    out 0x92, al

    ; --------------------------------------------------------
    ; REAL MODE MARKER B
    ; A20 enabled.
    ; --------------------------------------------------------

    call mark_B

    ; --------------------------------------------------------
    ; Load GDT
    ; --------------------------------------------------------

    lgdt [gdt_descriptor]

    ; --------------------------------------------------------
    ; REAL MODE MARKER C
    ; GDT loaded.
    ; --------------------------------------------------------

    call mark_C

    ; --------------------------------------------------------
    ; Enter protected mode
    ; --------------------------------------------------------

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; --------------------------------------------------------
    ; REAL MODE MARKER D
    ; PE bit is now set.
    ;
    ; We are still using the old code segment until the
    ; far jump below.
    ; --------------------------------------------------------

    call mark_D

    ; --------------------------------------------------------
    ; Far jump to 32-bit protected-mode code.
    ;
    ; 66 EA = 32-bit far jump
    ; DWORD = destination offset
    ; WORD  = code selector
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
; REAL MODE VGA MARKERS
; ============================================================

mark_A:
    push ax
    push es

    mov ax, 0xB800
    mov es, ax

    mov word [es:140], 0x0741

    pop es
    pop ax

    ret


mark_B:
    push ax
    push es

    mov ax, 0xB800
    mov es, ax

    mov word [es:142], 0x0742

    pop es
    pop ax

    ret


mark_C:
    push ax
    push es

    mov ax, 0xB800
    mov es, ax

    mov word [es:144], 0x0743

    pop es
    pop ax

    ret


mark_D:
    push ax
    push es

    mov ax, 0xB800
    mov es, ax

    mov word [es:146], 0x0744

    pop es
    pop ax

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
    ; Marker P
    ; Protected mode successfully entered.
    ; --------------------------------------------------------

    mov word [0xB8000], 0x0750


    ; --------------------------------------------------------
    ; Set flat data segments
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
    ; 45 × 512 = 23040 bytes
    ; 23040 / 4 = 5760 DWORDs
    ;
    ; 0x8000 -> 0x100000
    ; --------------------------------------------------------

    mov esi, 0x8000
    mov edi, 0x100000
    mov ecx, 5760

    rep movsd


    ; --------------------------------------------------------
    ; Marker C
    ; Kernel copied.
    ; --------------------------------------------------------

    mov word [0xB8002], 0x0743


    ; --------------------------------------------------------
    ; Marker J
    ; About to jump to kernel.
    ; --------------------------------------------------------

    mov word [0xB8004], 0x074A


    ; --------------------------------------------------------
    ; Jump to kernel entry
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

