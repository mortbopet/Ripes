.text
 main:


  test_2:
 la a1, tdat
 flw f1, 4(a1)
 fsw f1, 20(a1)
 lw a0, 20(a1)
 li x29, 0x40000000
 li gp, 2
 bne a0, x29, fail

  test_3:
 la a1, tdat
 flw f1, 0(a1)
 fsw f1, 24(a1)
 lw a0, 24(a1)
 li x29, 0xbf800000
 li gp, 3
 bne a0, x29, fail


  bne x0, gp, pass
 fail: li a0, 0
 li a7, 93
 ecall

 pass: li a0, 42
 li a7, 93
 ecall


.data

tdat:
.word 0xbf800000
.word 0x40000000
.word 0x40400000
.word 0xc0800000
.word 0xdeadbeef
.word 0xcafebabe
.word 0xabad1dea
.word 0x1337d00d


