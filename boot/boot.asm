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
    mov word [0xB8000], 0x074B

    cli

    mov esp, 0x90000
    and esp, -16
    mov ebp, 0

    mov word [0xB8002], 0x0743

    ; Normal CustOS boot = magic 0.
    push 0
    call kernel_main

.hang:
    cli
    hlt
    jmp .hang

section .note.GNU-stack noalloc noexec nowrite progbits
