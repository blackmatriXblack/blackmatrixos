[BITS 32]
section .text
[GLOBAL _idt_flush]

_idt_flush:
    mov eax, [esp + 4]
    lidt [eax]
    ret
