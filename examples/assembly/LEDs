# LEDs
# This program draws an animation on an LED matrix peripheral.
#
# To run this program, make sure that you have instantiated an "LED Matrix"
# peripheral in the "I/O" tab.


li a0 LED_MATRIX_0_BASE
li a1 LED_MATRIX_0_WIDTH
li a2 LED_MATRIX_0_HEIGHT

loop:
        addi    sp, sp, -16
        sw      s0, 12(sp)
        sw      s1, 8(sp)
        mv      t6, zero
        add     t3, a2, a1
        slli    a6, a1, 2
        lui     a3, 16
        addi    a7, a3, -256
        lui     t0, 4080
init:
        mv      t4, zero
        mv      t1, zero
        mv      t2, a0
nextRow:
        mv      a4, zero
        slli    a3, t1, 8
        sub     a3, a3, t1
        divu    a3, a3, a2
        add     a3, a3, t6
        slli    a3, a3, 8
        and     t5, a3, a7
        mv      a5, t2
        mv      a3, a1
nextPixel:
        divu    s0, a4, a1
        add     s0, s0, t6
        add     s1, t4, a4
        divu    s1, s1, t3
        add     s1, s1, t6
        slli    s0, s0, 16
        and     s0, s0, t0
        or      s0, t5, s0
        andi    s1, s1, 255
        or      s0, s0, s1
        sw      s0, 0(a5)
        addi    a3, a3, -1
        addi    a5, a5, 4
        addi    a4, a4, 255
        bnez    a3, nextPixel
        addi    t6, t6, 1
        addi    t1, t1, 1
        add     t2, t2, a6
        addi    t4, t4, 255
        bne     t1, a2, nextRow
        j       init
