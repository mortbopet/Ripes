
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
 fcvt.w.d a0, f0, rtz
 fsflags a1, x0
 li a2, 0x01
 bne a0, a3, fail
 bne a1, a2, fail
  .data
 .align 3
 test_2_data: .double -1.1
 .double 0.0
 .double 0.0
 .word -1
 .text

  test_3: li gp, 3
 la a0, test_3_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 fcvt.w.d a0, f0, rtz
 fsflags a1, x0
 li a2, 0x00
 bne a0, a3, fail
 bne a1, a2, fail
  .data
 .align 3
 test_3_data: .double -1.0
 .double 0.0
 .double 0.0
 .word -1
 .text

  test_4: li gp, 4
 la a0, test_4_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 fcvt.w.d a0, f0, rtz
 fsflags a1, x0
 li a2, 0x01
 bne a0, a3, fail
 bne a1, a2, fail
  .data
 .align 3
 test_4_data: .double -0.9
 .double 0.0
 .double 0.0
 .word 0
 .text

  test_5: li gp, 5
 la a0, test_5_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 fcvt.w.d a0, f0, rtz
 fsflags a1, x0
 li a2, 0x01
 bne a0, a3, fail
 bne a1, a2, fail
  .data
 .align 3
 test_5_data: .double 0.9
 .double 0.0
 .double 0.0
 .word 0
 .text

  test_6: li gp, 6
 la a0, test_6_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 fcvt.w.d a0, f0, rtz
 fsflags a1, x0
 li a2, 0x00
 bne a0, a3, fail
 bne a1, a2, fail
  .data
 .align 3
 test_6_data: .double 1.0
 .double 0.0
 .double 0.0
 .word 1
 .text

  test_7: li gp, 7
 la a0, test_7_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 fcvt.w.d a0, f0, rtz
 fsflags a1, x0
 li a2, 0x01
 bne a0, a3, fail
 bne a1, a2, fail
  .data
 .align 3
 test_7_data: .double 1.1
 .double 0.0
 .double 0.0
 .word 1
 .text

  test_8: li gp, 8
 la a0, test_8_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 fcvt.w.d a0, f0, rtz
 fsflags a1, x0
 li a2, 0x10
 bne a0, a3, fail
 bne a1, a2, fail
  .data
 .align 3
 test_8_data: .double -3e9
 .double 0.0
 .double 0.0
 .word 0x80000000
 .text

  test_9: li gp, 9
 la a0, test_9_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 fcvt.w.d a0, f0, rtz
 fsflags a1, x0
 li a2, 0x10
 bne a0, a3, fail
 bne a1, a2, fail
  .data
 .align 3
 test_9_data: .double 3e9
 .double 0.0
 .double 0.0
 .word 0x7FFFFFFF
 .text


  test_12: li gp, 12
 la a0, test_12_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 fcvt.wu.d a0, f0, rtz
 fsflags a1, x0
 li a2, 0x10
 bne a0, a3, fail
 bne a1, a2, fail
  .data
 .align 3
 test_12_data: .double -3.0
 .double 0.0
 .double 0.0
 .word 0
 .text

  test_13: li gp, 13
 la a0, test_13_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 fcvt.wu.d a0, f0, rtz
 fsflags a1, x0
 li a2, 0x10
 bne a0, a3, fail
 bne a1, a2, fail
  .data
 .align 3
 test_13_data: .double -1.0
 .double 0.0
 .double 0.0
 .word 0
 .text

  test_14: li gp, 14
 la a0, test_14_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 fcvt.wu.d a0, f0, rtz
 fsflags a1, x0
 li a2, 0x01
 bne a0, a3, fail
 bne a1, a2, fail
  .data
 .align 3
 test_14_data: .double -0.9
 .double 0.0
 .double 0.0
 .word 0
 .text

  test_15: li gp, 15
 la a0, test_15_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 fcvt.wu.d a0, f0, rtz
 fsflags a1, x0
 li a2, 0x01
 bne a0, a3, fail
 bne a1, a2, fail
  .data
 .align 3
 test_15_data: .double 0.9
 .double 0.0
 .double 0.0
 .word 0
 .text

  test_16: li gp, 16
 la a0, test_16_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 fcvt.wu.d a0, f0, rtz
 fsflags a1, x0
 li a2, 0x00
 bne a0, a3, fail
 bne a1, a2, fail
  .data
 .align 3
 test_16_data: .double 1.0
 .double 0.0
 .double 0.0
 .word 1
 .text

  test_17: li gp, 17
 la a0, test_17_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 fcvt.wu.d a0, f0, rtz
 fsflags a1, x0
 li a2, 0x01
 bne a0, a3, fail
 bne a1, a2, fail
  .data
 .align 3
 test_17_data: .double 1.1
 .double 0.0
 .double 0.0
 .word 1
 .text

  test_18: li gp, 18
 la a0, test_18_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 fcvt.wu.d a0, f0, rtz
 fsflags a1, x0
 li a2, 0x10
 bne a0, a3, fail
 bne a1, a2, fail
  .data
 .align 3
 test_18_data: .double -3e9
 .double 0.0
 .double 0.0
 .word 0
 .text

  test_19: li gp, 19
 la a0, test_19_data 
 fld f0, 0(a0)
 fld f1, 8(a0)
 fld f2, 16(a0)
 lw a3, 24(a0)
 fcvt.wu.d a0, f0, rtz
 fsflags a1, x0
 li a2, 0x00
 bne a0, a3, fail
 bne a1, a2, fail
  .data
 .align 3
 test_19_data: .double 3e9
 .double 0.0
 .double 0.0
 .word  0xb2d05e00

 .text

# 60 "isa/rv32ud/../rv64ud/fcvt_w.S"
  # test negative NaN, negative infinity conversion
  test_42: la x1, tdat_d
 fld f1, 0(x1)
 fcvt.w.d x1, f1
 li x29, 0x7fffffff
 li gp, 42
 bne x1, x29, fail

  test_44: la x1, tdat_d
 fld f1, 16(x1)
 fcvt.w.d x1, f1
 li x29, 0x80000000
 li gp, 44
 bne x1, x29, fail


  # test positive NaN, positive infinity conversion
  test_52: la x1, tdat_d
 fld f1, 8(x1)
 fcvt.w.d x1, f1
 li x29, 0x7fffffff
 li gp, 52
 bne x1, x29, fail




  test_54: la x1, tdat_d
 fld f1, 24(x1)
 fcvt.w.d x1, f1
 li x29, 0x7fffffff
 li gp, 54
 bne x1, x29, fail





  # test NaN, infinity conversions to unsigned integer
  test_62: la x1, tdat_d
 fld f1, 0(x1)
 fcvt.wu.d x1, f1
 li x29, -1
 li gp, 62
 bne x1, x29, fail

  test_63: la x1, tdat_d
 fld f1, 8(x1)
 fcvt.wu.d x1, f1
 li x29, -1
 li gp, 63
 bne x1, x29, fail

  test_64: la x1, tdat_d
 fld f1, 16(x1)
 fcvt.wu.d x1, f1
 li x29, 0
 li gp, 64
 bne x1, x29, fail

  test_65: la x1, tdat_d
 fld f1, 24(x1)
 fcvt.wu.d x1, f1
 li x29, -1
 li gp, 65
 bne x1, x29, fail

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
 # -NaN, NaN, -inf, +inf
tdat:
.word 0xffffffff
.word 0x7fffffff
.word 0xff800000
.word 0x7f800000

tdat_d:
.word -1, -1
.word -1, 0x7fffffff
.word 0, 0xfff00000
.word 0, 0x7ff00000
