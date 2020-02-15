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
 fmin.d f3, f0, f1
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
 test_2_data: .double 2.5
 .double 1.0
 .double 0.0
 .double 1.0
 .text

  test_3: li gp, 3
 la a0, test_3_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 fmin.d f3, f0, f1
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
 test_3_data: .double -1235.1
 .double 1.1
 .double 0.0
 .double -1235.1
 .text

  test_4: li gp, 4
 la a0, test_4_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 fmin.d f3, f0, f1
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
 test_4_data: .double 1.1
 .double -1235.1
 .double 0.0
 .double -1235.1
 .text

  test_5: li gp, 5
 la a0, test_5_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 fmin.d f3, f0, f1
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
 test_5_data: .double NaN
 .double -1235.1
 .double 0.0
 .double -1235.1
 .text

  test_6: li gp, 6
 la a0, test_6_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 fmin.d f3, f0, f1
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
 test_6_data: .double 3.14159265
 .double 0.00000001
 .double 0.0
 .double 0.00000001
 .text

  test_7: li gp, 7
 la a0, test_7_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 fmin.d f3, f0, f1
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
 test_7_data: .double -1.0
 .double -2.0
 .double 0.0
 .double -2.0
 .text


  test_12: li gp, 12
 la a0, test_12_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 fmax.d f3, f0, f1
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
 test_12_data: .double 2.5
 .double 1.0
 .double 0.0
 .double 2.5
 .text

  test_13: li gp, 13
 la a0, test_13_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 fmax.d f3, f0, f1
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
 test_13_data: .double -1235.1
 .double 1.1
 .double 0.0
 .double 1.1
 .text

  test_14: li gp, 14
 la a0, test_14_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 fmax.d f3, f0, f1
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
 test_14_data: .double 1.1
 .double -1235.1
 .double 0.0
 .double 1.1
 .text

  test_15: li gp, 15
 la a0, test_15_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 fmax.d f3, f0, f1
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
 test_15_data: .double NaN
 .double -1235.1
 .double 0.0
 .double -1235.1
 .text

  test_16: li gp, 16
 la a0, test_16_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 fmax.d f3, f0, f1
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
 test_16_data: .double 3.14159265
 .double 0.00000001
 .double 0.0
 .double 3.14159265
 .text

  test_17: li gp, 17
 la a0, test_17_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 fmax.d f3, f0, f1
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
 test_17_data: .double -1.0
 .double -2.0
 .double 0.0
 .double -1.0
 .text


  # FMIN(0d:7ff0000000000001, x) = x
  test_20: li gp, 20
 la a0, test_20_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 fmax.d f3, f0, f1
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
 test_20_data: .word 1, 0x7ff00000
 .double 1.0
 .double 0.0
 .double 1.0
 .text

  # FMIN(0d:7ff8000000000000, 0d:7ff8000000000000) = canonical NaN
  test_21: li gp, 21
 la a0, test_21_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 fmax.d f3, f0, f1
 fsd f3, 0(a0)
 lw t2, 4(a0)
 lw a0, 0(a0)
 fsflags a1, x0
 li a2, 0x00
 bne a0, a3, fail
 bne t1, t2, fail
 bne a1, a2, fail
  .data
 .align 3
 test_21_data: .double NaN
 .double NaN
 .double 0.0
.word 0, 0x7ff80000
 .text


  # -0.0 < +0.0
  test_30: li gp, 30
 la a0, test_30_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 fmin.d f3, f0, f1
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
 test_30_data: .double -0.0
 .double 0.0
 .double 0.0
 .double -0.0
 .text

  test_31: li gp, 31
 la a0, test_31_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 fmin.d f3, f0, f1
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
 test_31_data: .double 0.0
 .double -0.0
 .double 0.0
 .double -0.0
 .text

  test_32: li gp, 32
 la a0, test_32_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 fmax.d f3, f0, f1
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
 test_32_data: .double -0.0
 .double 0.0
 .double 0.0
 .double 0.0
 .text

  test_33: li gp, 33
 la a0, test_33_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 fmax.d f3, f0, f1
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
 test_33_data: .double 0.0
 .double -0.0
 .double 0.0
 .double 0.0
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
