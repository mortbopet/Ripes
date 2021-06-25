.text
_reset:
j _start

.align 16
_irq_vec:
    j _reset
    
_start:
    nop
    j _reset
