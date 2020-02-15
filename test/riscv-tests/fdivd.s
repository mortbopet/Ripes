.text
  #-------------------------------------------------------------
  # Arithmetic tests
  #-------------------------------------------------------------

  test_2: li gp, 2
 la a0, test_2_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 fdiv.d f3, f0, f1
 fsd f3, 0(a0)
 lw t2, 4(a0)
 lw a0, 0(a0)
 fsflags a1, x0
 li a2, 1
 bne a0, a3, fail
 bne t1, t2, fail
 bne a1, a2, fail
  .data
 .align 3
 test_2_data: .double 3.14159265
 .double 2.71828182
 .double 0.0
 .double 1.1557273520668288
 .text

  test_3: li gp, 3
 la a0, test_3_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 fdiv.d f3, f0, f1
 fsd f3, 0(a0)
 lw t2, 4(a0)
 lw a0, 0(a0)
 fsflags a1, x0
 li a2, 1
 bne a0, a3, fail
 bne t1, t2, fail
 bne a1, a2, fail
  .data
 .align 3
 test_3_data: .double -1234
 .double 1235.1
 .double 0.0
 .double -0.9991093838555584
 .text

  test_4: li gp, 4
 la a0, test_4_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 fdiv.d f3, f0, f1
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
 test_4_data: .double 3.14159265
 .double 1.0
 .double 0.0
 .double 3.14159265
 .text


  test_5: li gp, 5
 la a0, test_5_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 fsqrt.d f3, f0
 fsd f3, 0(a0)
 lw t2, 4(a0)
 lw a0, 0(a0)
 fsflags a1, x0
 li a2, 1
 bne a0, a3, fail
 bne t1, t2, fail
 bne a1, a2, fail
  .data
 .align 3
 test_5_data: .double 3.14159265
 .double 0.0
 .double 0.0
 .double 1.7724538498928541
 .text

  test_6: li gp, 6
 la a0, test_6_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 fsqrt.d f3, f0
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
 test_6_data: .double 10000
 .double 0.0
 .double 0.0
 .double 100
 .text


  test_16: li gp, 16
 la a0, test_16_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 fsqrt.d f3, f0
 fsd f3, 0(a0)
 lw t2, 4(a0)
 lw a0, 0(a0)
 fsflags a1, x0
 li a2, 0x10
 bne a0, a3, fail
 bne t1, t2, fail
 bne a1, a2, fail
  .data
 .align 3
 test_16_data: .double -1.0
 .double 0.0
 .double 0.0
 .word 0, 0x7FF80000
 .text


  test_7: li gp, 7
 la a0, test_7_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 fsqrt.d f3, f0
 fsd f3, 0(a0)
 lw t2, 4(a0)
 lw a0, 0(a0)
 fsflags a1, x0
 li a2, 1
 bne a0, a3, fail
 bne t1, t2, fail
 bne a1, a2, fail
  .data
 .align 3
 test_7_data: .double 171.0
 .double 0.0
 .double 0.0
 .double 13.076696830622021
 .text


  test_8: li gp, 8
 la a0, test_8_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 fsqrt.d f3, f0
 fsd f3, 0(a0)
 lw t2, 4(a0)
 lw a0, 0(a0)
 fsflags a1, x0
 li a2, 1
 bne a0, a3, fail
 bne t1, t2, fail
 bne a1, a2, fail
  .data
 .align 3
 test_8_data: .double 1.60795e-7
 .double 0.0
 .double 0.0
 .double 0.00040099251863345283320230749702
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
