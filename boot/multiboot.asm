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
; KERNEL ENTRY
; ============================================================

section .text

global _start

extern kernel_main
extern boot_magic

_start:

    cli

    ; --------------------------------------------------------
    ; Save the Multiboot magic passed by GRUB.
    ;
    ; GRUB places 0x2BADB002 in EAX.
    ; kernel.c uses this to detect installer mode.
    ; --------------------------------------------------------

    mov [boot_magic], eax


    ; --------------------------------------------------------
    ; Set up installer kernel stack
    ; --------------------------------------------------------

    mov esp, 0x90000
    and esp, -16

    mov ebp, 0


    ; --------------------------------------------------------
    ; Start shared kernel
    ; --------------------------------------------------------

    call kernel_main


; ============================================================
; HANG
; ============================================================

.hang:
    cli
    hlt
    jmp .hang


; ============================================================
; GNU STACK NOTE
; ============================================================

section .note.GNU-stack noalloc noexec nowrite progbits

