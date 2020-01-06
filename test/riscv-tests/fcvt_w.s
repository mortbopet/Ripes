.text
 main:


  #-------------------------------------------------------------
  # Arithmetic tests
  #-------------------------------------------------------------


  test_2: li gp, 2
 la a0, test_2_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fcvt.w.s a0, f0, rtz
 fsflags a1, x0
 li a2, 0x01
 bne a0, a3, fail
 bne a1, a2, fail

  test_3: li gp, 3
 la a0, test_3_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fcvt.w.s a0, f0, rtz
 fsflags a1, x0
 li a2, 0x00
 bne a0, a3, fail
 bne a1, a2, fail

  test_4: li gp, 4
 la a0, test_4_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fcvt.w.s a0, f0, rtz
 fsflags a1, x0
 li a2, 0x01
 bne a0, a3, fail
 bne a1, a2, fail

  test_5: li gp, 5
 la a0, test_5_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fcvt.w.s a0, f0, rtz
 fsflags a1, x0
 li a2, 0x01
 bne a0, a3, fail
 bne a1, a2, fail

  test_6: li gp, 6
 la a0, test_6_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fcvt.w.s a0, f0, rtz
 fsflags a1, x0
 li a2, 0x00
 bne a0, a3, fail
 bne a1, a2, fail

  test_7: li gp, 7
 la a0, test_7_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fcvt.w.s a0, f0, rtz
 fsflags a1, x0
 li a2, 0x01
 bne a0, a3, fail
 bne a1, a2, fail

  test_8: li gp, 8
 la a0, test_8_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fcvt.w.s a0, f0, rtz
 fsflags a1, x0
 li a2, 0x10
 bne a0, a3, fail
 bne a1, a2, fail

  test_9: li gp, 9
 la a0, test_9_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fcvt.w.s a0, f0, rtz
 fsflags a1, x0
 li a2, 0x10
 bne a0, a3, fail
 bne a1, a2, fail


  test_12: li gp, 12
 la a0, test_12_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fcvt.wu.s a0, f0, rtz
 fsflags a1, x0
 li a2, 0x10
 bne a0, a3, fail
 bne a1, a2, fail
 
  test_13: li gp, 13
 la a0, test_13_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fcvt.wu.s a0, f0, rtz
 fsflags a1, x0
 li a2, 0x10
 bne a0, a3, fail
 bne a1, a2, fail
 
  test_14: li gp, 14
 la a0, test_14_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fcvt.wu.s a0, f0, rtz
 fsflags a1, x0
 li a2, 0x01
 bne a0, a3, fail
 bne a1, a2, fail
 
  test_15: li gp, 15
 la a0, test_15_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fcvt.wu.s a0, f0, rtz
 fsflags a1, x0
 li a2, 0x01
 bne a0, a3, fail
 bne a1, a2, fail
 
  test_16: li gp, 16
 la a0, test_16_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fcvt.wu.s a0, f0, rtz
 fsflags a1, x0
 li a2, 0x00
 bne a0, a3, fail
 bne a1, a2, fail
 
  test_17: li gp, 17
 la a0, test_17_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fcvt.wu.s a0, f0, rtz
 fsflags a1, x0
 li a2, 0x01
 bne a0, a3, fail
 bne a1, a2, fail
 
  test_18: li gp, 18
 la a0, test_18_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fcvt.wu.s a0, f0, rtz
 fsflags a1, x0
 li a2, 0x10
 bne a0, a3, fail
 bne a1, a2, fail
 
  test_19: li gp, 19
 la a0, test_19_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fcvt.wu.s a0, f0, rtz
 fsflags a1, x0
 li a2, 0x00
 bne a0, a3, fail
 bne a1, a2, fail
 
# 55 "rv32uf/../rv64uf/fcvt_w.S"
  # test negative NaN, negative infinity conversion
  test_42:
 la x1, tdat 
 flw f1, 0(x1)
 fcvt.w.s x1, f1
 li x29, 0x7fffffff
 li gp, 42
 bne x1, x29, fail

  test_44:
 la x1, tdat 
 flw f1, 8(x1)
 fcvt.w.s x1, f1
 li x29, 0x80000000
 li gp, 44
 bne x1, x29, fail


  # test positive NaN, positive infinity conversion
  test_52:
 la x1, tdat 
 flw f1, 4(x1)
 fcvt.w.s x1, f1
 li x29, 0x7fffffff
 li gp, 52
 bne x1, x29, fail

  test_54:
 la x1, tdat 
 flw f1, 12(x1)
 fcvt.w.s x1, f1
 li x29, 0x7fffffff
 li gp, 54
 bne x1, x29, fail


  # test NaN, infinity conversions to unsigned integer
  test_62:
 la x1, tdat 
 flw f1, 0(x1)
 fcvt.wu.s x1, f1
 li x29, 0xffffffff
 li gp, 62
 bne x1, x29, fail

  test_63:
 la x1, tdat 
 flw f1, 4(x1)
 fcvt.wu.s x1, f1
 li x29, 0xffffffff
 li gp, 63
 bne x1, x29, fail

  test_64:
 la x1, tdat 
 flw f1, 8(x1)
 fcvt.wu.s x1, f1
 li x29, 0
 li gp, 64
 bne x1, x29, fail

  test_65:
 la x1, tdat 
 flw f1, 12(x1)
 fcvt.wu.s x1, f1
 li x29, 0xffffffff
 li gp, 65
 bne x1, x29, fail


  bne x0, gp, pass
 fail: li a0, 0
 li a7, 93
 ecall

 pass: li a0, 42
 li a7, 93
 ecall


.data

 test_2_data: .float -1.1
 .float 0.0
 .float 0.0
 .word -1
 
 test_3_data: .float -1.0
 .float 0.0
 .float 0.0
 .word -1
 
 test_4_data: .float -0.9
 .float 0.0
 .float 0.0
 .word 0
 
 test_5_data: .float 0.9
 .float 0.0
 .float 0.0
 .word 0
 
 test_6_data: .float 1.0
 .float 0.0
 .float 0.0
 .word 1
 
 test_7_data: .float 1.1
 .float 0.0
 .float 0.0
 .word 1
 
 test_8_data: .float -3e9
 .float 0.0
 .float 0.0
 .word 0x80000000
 
 test_9_data: .float 3e9
 .float 0.0
 .float 0.0
 .word 0x7fffffff
 

 test_12_data: .float -3.0
 .float 0.0
 .float 0.0
 .word 0
 
 test_13_data: .float -1.0
 .float 0.0
 .float 0.0
 .word 0
 
 test_14_data: .float -0.9
 .float 0.0
 .float 0.0
 .word 0
 
 test_15_data: .float 0.9
 .float 0.0
 .float 0.0
 .word 0
 
 test_16_data: .float 1.0
 .float 0.0
 .float 0.0
 .word 1
 
 test_17_data: .float 1.1
 .float 0.0
 .float 0.0
 .word 1
 
 test_18_data: .float -3e9
 .float 0.0
 .float 0.0
 .word 0
 
 test_19_data: .float 3e9
 .float 0.0
 .float 0.0
 .word 3000000000
 

 # -NaN, NaN, -inf, +inf
tdat:
.word 0xffffffff
.word 0x7fffffff
.word 0xff800000
.word 0x7f800000

tdat_d:
.word 0xffffffff, 0xffffffff
.word 0x7fffffff, 0xffffffff
.word 0xfff00000, 0x00000000
.word 0x7ff00000, 0x00000000
