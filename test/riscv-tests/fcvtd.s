.text

  #-------------------------------------------------------------
  # Arithmetic tests
  #-------------------------------------------------------------

  test_2: li gp, 2
 la a0, test_2_data 
 lw a3, 0(a0)
 lw a4, 4(a0)
 li a1, 2
 fcvt.d.w f0, a1
 fsd f0, 0(a0)
 lw a1, 4(a0)
 lw a0, 0(a0)
 fsflags x0
 bne a0, a3, fail
 bne a1, a4, fail
  .data
 .align 3
 test_2_data: .double 2.0
 .text

  test_3: li gp, 3
 la a0, test_3_data 
 lw a3, 0(a0)
 lw a4, 4(a0)
 li a1, -2
 fcvt.d.w f0, a1
 fsd f0, 0(a0)
 lw a1, 4(a0)
 lw a0, 0(a0)
 fsflags x0
 bne a0, a3, fail
 bne a1, a4, fail
  .data
 .align 3
 test_3_data: .double -2.0
 .text


  test_4: li gp, 4
 la a0, test_4_data 
 lw a3, 0(a0)
 lw a4, 4(a0)
 li a1, 2
 fcvt.d.wu f0, a1
 fsd f0, 0(a0)
 lw a1, 4(a0)
 lw a0, 0(a0)
 fsflags x0
 bne a0, a3, fail
 bne a1, a4, fail
  .data
 .align 3
 test_4_data: .double 2.0
 .text

  test_5: li gp, 5
 la a0, test_5_data 
 lw a3, 0(a0)
 lw a4, 4(a0)
 li a1, -2
 fcvt.d.wu f0, a1
 fsd f0, 0(a0)
 lw a1, 4(a0)
 lw a0, 0(a0)
 fsflags x0
 bne a0, a3, fail
 bne a1, a4, fail
  .data
 .align 3
 test_5_data: .double 4294967294
 .text

# 43 "isa/rv32ud/../rv64ud/fcvt.S"
  test_10: li gp, 10
 la a0, test_10_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 fcvt.s.d f3, f0
 fcvt.d.s f3, f3
 fsd f3, 0(a0)
 lw t2, 4(a0)
 lw a0, 0(a0)
 fsflags a1, x0
 li a2, 0
 bne a0, a3, fail
 bne t1, t2, fail
 bne a1, a2, fail
  .data
 .align 3
 test_10_data: .double -1.5
 .double 0.0
 .double 0.0
 .double -1.5
 .text
  test_11: li gp, 11
 la a0, test_11_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fcvt.d.s f3, f0
 fcvt.s.d f3, f3
 fmv.x.s a0, f3
 fsflags a1, x0
 li a2, 0
 bne a0, a3, fail
 bne a1, a2, fail
  .data
 .align 2
 test_11_data: .float -1.5
 .float 0.0
 .float 0.0
 .float -1.5
 .text
# 56 "isa/rv32ud/../rv64ud/fcvt.S"
  test_12: la a1, test_data_22
 fld f2, 0(a1)
 fcvt.s.d f2, f2
 fcvt.d.s f2, f2
 fsd f2, 0(a1)
 lw a0, 0(a1)
 lw a1, 4(a1)
 la x15, test_12_data 
 lw x29, 0(x15)
 lw x15, 4(x15)
 li gp, 12
 bne a0, x29, fail
 bne a1, x15, fail
  .data
 .align 3
 test_12_data: .word 0, 0x7ff80000
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
test_data_22:
  .word  0xffff8004, 0x7ffcffff
