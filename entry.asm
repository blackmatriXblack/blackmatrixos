[BITS 32]
[GLOBAL _start]

section .text

_start:
    mov esp, stack_top
    extern kernel_main
    call kernel_main
    cli
.hang:
    hlt
    jmp .hang

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:
