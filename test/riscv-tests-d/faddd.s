
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
 fadd.d f3, f0, f1
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
 .double 3.5
 
 .text

  test_3: li gp, 3
 la a0, test_3_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 fadd.d f3, f0, f1
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
 test_3_data: .double -1235.1
 .double 1.1
 .double 0.0
 .double -1234
 .text

  test_4: li gp, 4
 la a0, test_4_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 fadd.d f3, f0, f1
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
 test_4_data: .double 3.14159265
 .double 0.00000001
 .double 0.0
 .double 3.14159266
 .text


  test_5: li gp, 5
 la a0, test_5_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 fsub.d f3, f0, f1
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
 test_5_data: .double 2.5
 .double 1.0
 .double 0.0
 .double 1.5
 .text

  test_6: li gp, 6
 la a0, test_6_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 fsub.d f3, f0, f1
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
 test_6_data: .double -1235.1
 .double -1.1
 .double 0.0
 .double -1234
 .text

  test_7: li gp, 7
 la a0, test_7_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 fsub.d f3, f0, f1
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
 test_7_data: .double 3.14159265
 .double 0.00000001
 .double 0.0
 .double 3.1415926400000001
 .text


  test_8: li gp, 8
 la a0, test_8_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 fmul.d f3, f0, f1
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
 test_8_data: .double 2.5
 .double 1.0
 .double 0.0
 .double 2.5
 .text

  test_9: li gp, 9
 la a0, test_9_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 fmul.d f3, f0, f1
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
 test_9_data: .double -1235.1
 .double -1.1
 .double 0.0
 .double 1358.61
 .text

  test_10: li gp, 10
 la a0, test_10_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 fmul.d f3, f0, f1
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
 test_10_data: .double 3.14159265
 .double 0.00000001
 .double 0.0
 .double 3.14159265e-8
 .text


  # Is the canonical NaN generated for Inf - Inf?
  test_11: li gp, 11
 la a0, test_11_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 fsub.d f3, f0, f1
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
 test_11_data: .double Inf
 .double Inf
 .double 0.0
 .double NaN
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
