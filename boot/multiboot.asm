bits 32

section .multiboot
align 4

multiboot_header:
    dd 0x1BADB002
    dd 0
    dd -(0x1BADB002)

section .text

global _start
extern kernel_main

_start:
    cli

    mov esp, 0x90000
    and esp, -16
    mov ebp, 0

    ; GRUB gives us the Multiboot magic in EAX.
    push eax
    call kernel_main

.hang:
    cli
    hlt
    jmp .hang

section .note.GNU-stack noalloc noexec nowrite progbits
