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
 feq.d a0, f0, f1
 li t2, 0
 fsflags a1, x0
 li a2, 0x00
 bne a0, a3, fail
 bne t1, t2, fail
 bne a1, a2, fail
  .data
 .align 3
 test_2_data: .double -1.36
 .double -1.36
 .double 0.0
 .word 1,0
 .text
  test_3: li gp, 3
 la a0, test_3_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 fle.d a0, f0, f1
 li t2, 0
 fsflags a1, x0
 li a2, 0x00
 bne a0, a3, fail
 bne t1, t2, fail
 bne a1, a2, fail
  .data
 .align 3
 test_3_data: .double -1.36
 .double -1.36
 .double 0.0
 .word 1,0
 .text
  test_4: li gp, 4
 la a0, test_4_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 flt.d a0, f0, f1
 li t2, 0
 fsflags a1, x0
 li a2, 0x00
 bne a0, a3, fail
 bne t1, t2, fail
 bne a1, a2, fail
  .data
 .align 3
 test_4_data: .double -1.36
 .double -1.36
 .double 0.0
 .word 0,0
 .text

  test_5: li gp, 5
 la a0, test_5_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 feq.d a0, f0, f1
 li t2, 0
 fsflags a1, x0
 li a2, 0x00
 bne a0, a3, fail
 bne t1, t2, fail
 bne a1, a2, fail
  .data
 .align 3
 test_5_data: .double -1.37
 .double -1.36
 .double 0.0
 .word 0,0
 .text
  test_6: li gp, 6
 la a0, test_6_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 fle.d a0, f0, f1
 li t2, 0
 fsflags a1, x0
 li a2, 0x00
 bne a0, a3, fail
 bne t1, t2, fail
 bne a1, a2, fail
  .data
 .align 3
 test_6_data: .double -1.37
 .double -1.36
 .double 0.0
 .word 1,0
 .text
  test_7: li gp, 7
 la a0, test_7_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 flt.d a0, f0, f1
 li t2, 0
 fsflags a1, x0
 li a2, 0x00
 bne a0, a3, fail
 bne t1, t2, fail
 bne a1, a2, fail
  .data
 .align 3
 test_7_data: .double -1.37
 .double -1.36
 .double 0.0
 .word 1,0
 .text

  # Only 0d:7ff0000000000001 should signal invalid for feq.
  test_8: li gp, 8
 la a0, test_8_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 feq.d a0, f0, f1
 li t2, 0
 fsflags a1, x0
 li a2, 0x00
 bne a0, a3, fail
 bne t1, t2, fail
 bne a1, a2, fail
  .data
 .align 3
 test_8_data: .double NaN
 .double 0
 .double 0.0
 .word 0,0
 .text
  test_9: li gp, 9
 la a0, test_9_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 feq.d a0, f0, f1
 li t2, 0
 fsflags a1, x0
 li a2, 0x00
 bne a0, a3, fail
 bne t1, t2, fail
 bne a1, a2, fail
  .data
 .align 3
 test_9_data: .double NaN
 .double NaN
 .double 0.0
 .word 0,0
 .text
  test_10: li gp, 10
 la a0, test_10_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 feq.d a0, f0, f1
 li t2, 0
 fsflags a1, x0
 li a2, 0x10
 bne a0, a3, fail
 bne t1, t2, fail
 bne a1, a2, fail
  .data
 .align 3
 test_10_data: .word 0x00000001, 0x7ff00000
 .double 0
 .double 0.0
 .word 0,0
 .text

  # 0d:7ff8000000000000 should signal invalid for fle/flt.
  test_11: li gp, 11
 la a0, test_11_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 flt.d a0, f0, f1
 li t2, 0
 fsflags a1, x0
 li a2, 0x10
 bne a0, a3, fail
 bne t1, t2, fail
 bne a1, a2, fail
  .data
 .align 3
 test_11_data: .double NaN
 .double 0
 .double 0.0
 .word 0,0
 .text
  test_12: li gp, 12
 la a0, test_12_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 flt.d a0, f0, f1
 li t2, 0
 fsflags a1, x0
 li a2, 0x10
 bne a0, a3, fail
 bne t1, t2, fail
 bne a1, a2, fail
  .data
 .align 3
 test_12_data: .double NaN
 .double NaN
 .double 0.0
 .word 0,0
 .text
  test_13: li gp, 13
 la a0, test_13_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 flt.d a0, f0, f1
 li t2, 0
 fsflags a1, x0
 li a2, 0x10
 bne a0, a3, fail
 bne t1, t2, fail
 bne a1, a2, fail
  .data
 .align 3
 test_13_data: .word 0x00000001, 0x7ff00000
 .double 0
 .double 0.0
 .word 0,0
 .text
  test_14: li gp, 14
 la a0, test_14_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 fle.d a0, f0, f1
 li t2, 0
 fsflags a1, x0
 li a2, 0x10
 bne a0, a3, fail
 bne t1, t2, fail
 bne a1, a2, fail
  .data
 .align 3
 test_14_data: .double NaN
 .double 0
 .double 0.0
 .word 0,0
 .text
  test_15: li gp, 15
 la a0, test_15_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 fle.d a0, f0, f1
 li t2, 0
 fsflags a1, x0
 li a2, 0x10
 bne a0, a3, fail
 bne t1, t2, fail
 bne a1, a2, fail
  .data
 .align 3
 test_15_data: .double NaN
 .double NaN
 .double 0.0
 .word 0,0
 .text
  test_16: li gp, 16
 la a0, test_16_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 lw t1, 28(a0)
 fle.d a0, f0, f1
 li t2, 0
 fsflags a1, x0
 li a2, 0x10
 bne a0, a3, fail
 bne t1, t2, fail
 bne a1, a2, fail
  .data
 .align 3
 test_16_data: .word 0x00000001, 0x7ff00000
 .double 0
 .double 0.0
 .word 0,0
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
