bits 32


; ============================================================
; MULTIBOOT HEADER
; ============================================================

section .multiboot

align 4

multiboot_header:

    dd 0x1BADB002
    dd 0
    dd -(0x1BADB002)


; ============================================================
; INSTALLER ENTRY
; ============================================================

section .text

global _start

extern kernel_installer_main


_start:

    cli


    ; --------------------------------------------------------
    ; Installer stack
    ; --------------------------------------------------------

    mov esp, 0x90000

    and esp, -16

    mov ebp, 0


    ; --------------------------------------------------------
    ; Start installer kernel
    ; --------------------------------------------------------

    call kernel_installer_main


.hang:

    cli
    hlt

    jmp .hang


section .note.GNU-stack noalloc noexec nowrite progbits

