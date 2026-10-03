bits 32

section .bss
align 16

stack_bottom:
    resb 16384
stack_top:

section .text
global _start
extern kernel_main

_start:
    cli

    ; Set up stack
    mov esp, stack_top

    ; Align stack for C
    and esp, -16

    ; Clear base pointer
    mov ebp, 0

    ; Start the C kernel
    call kernel_main

.hang:
    cli
    hlt
    jmp .hang

section .note.GNU-stack noalloc noexec nowrite progbits

