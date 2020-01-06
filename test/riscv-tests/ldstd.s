.text

  la s0, tdat
  test_2: fld f2, 0(s0)
 fsd f2, 16(s0)
 lw a0, 16(s0)
 lw a1, 20(s0)
 la x15, test_2_data 
 lw x29, 0(x15)
 lw x15, 4(x15)
 li gp, 2
 bne a0, x29, fail
 bne a1, x15, fail
  .data
 .align 3
 test_2_data: .word 0xbf800000, 0x40000000
 .text
  test_3: fld f2, 0(s0)
 fsw f2, 16(s0)
 lw a0, 16(s0)
 lw a1, 20(s0)
 la x15, test_3_data 
 lw x29, 0(x15)
 lw x15, 4(x15)
 li gp, 3
 bne a0, x29, fail
 bne a1, x15, fail
  .data
 .align 3
 test_3_data: .word 0xbf800000, 0x40000000
 .text
  test_4: flw f2, 0(s0)
 fsw f2, 16(s0)
 lw a0, 16(s0)
 lw a1, 20(s0)
 la x15, test_4_data 
 lw x29, 0(x15)
 lw x15, 4(x15)
 li gp, 4
 bne a0, x29, fail
 bne a1, x15, fail
  .data
 .align 3
 test_4_data: .word 0xbf800000, 0x40000000
 .text
  test_5: fld f2, 8(s0)
 fsd f2, 16(s0)
 lw a0, 16(s0)
 lw a1, 20(s0)
 la x15, test_5_data 
 lw x29, 0(x15)
 lw x15, 4(x15)
 li gp, 5
 bne a0, x29, fail
 bne a1, x15, fail
  .data
 .align 3
 test_5_data: .word 0x40400000, 0xc0800000
 .text
  test_6: flw f2, 8(s0)
 fsd f2, 16(s0)
 lw a0, 16(s0)
 lw a1, 20(s0)
 la x15, test_6_data 
 lw x29, 0(x15)
 lw x15, 4(x15)
 li gp, 6
 bne a0, x29, fail
 bne a1, x15, fail
  .data
 .align 3
 test_6_data: .word 0x40400000, 0xffffffff
 .text
	j success
fail:
	li a0, 0
	li a7, 93 
	ecall
	
success:
	li a0, 42 
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