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
 feq.s a0, f0, f1
 fsflags a1, x0
 li a2, 0x00
 bne a0, a3, fail
 bne a1, a2, fail
 
  test_3: li gp, 3
 la a0, test_3_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fle.s a0, f0, f1
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
 flt.s a0, f0, f1
 fsflags a1, x0
 li a2, 0x00
 bne a0, a3, fail
 bne a1, a2, fail
 

  test_5: li gp, 5
 la a0, test_5_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 feq.s a0, f0, f1
 fsflags a1, x0
 li a2, 0x00
 bne a0, a3, fail
 bne a1, a2, fail
 
  test_6: li gp, 6
 la a0, test_6_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fle.s a0, f0, f1
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
 flt.s a0, f0, f1
 fsflags a1, x0
 li a2, 0x00
 bne a0, a3, fail
 bne a1, a2, fail
 

  # Only 0d:7ff0000000000001 should signal invalid for feq.
  test_8: li gp, 8
 la a0, test_8_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 feq.s a0, f0, f1
 fsflags a1, x0
 li a2, 0x00
 bne a0, a3, fail
 bne a1, a2, fail
 
  test_9: li gp, 9
 la a0, test_9_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 feq.s a0, f0, f1
 fsflags a1, x0
 li a2, 0x00
 bne a0, a3, fail
 bne a1, a2, fail
 
  test_10: li gp, 10
 la a0, test_10_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 feq.s a0, f0, f1
 fsflags a1, x0
 li a2, 0x10
 bne a0, a3, fail
 bne a1, a2, fail


  # 0d:7ff8000000000000 should signal invalid for fle/flt.
  test_11: li gp, 11
 la a0, test_11_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 flt.s a0, f0, f1
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
 flt.s a0, f0, f1
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
 flt.s a0, f0, f1
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
 fle.s a0, f0, f1
 fsflags a1, x0
 li a2, 0x10
 bne a0, a3, fail
 bne a1, a2, fail

  test_15: li gp, 15
 la a0, test_15_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fle.s a0, f0, f1
 fsflags a1, x0
 li a2, 0x10
 bne a0, a3, fail
 bne a1, a2, fail

  test_16: li gp, 16
 la a0, test_16_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fle.s a0, f0, f1
 fsflags a1, x0
 li a2, 0x10
 bne a0, a3, fail
 bne a1, a2, fail


  bne x0, gp, pass
 fail: li a0, 0
 li a7, 93
 ecall

 pass: li a0, 42
 li a7, 93
 ecall


.data
 test_2_data: .float -1.36
 .float -1.36
 .float 0.0
 .word 1

 test_3_data: .float -1.36
 .float -1.36
 .float 0.0
 .word 1

 test_4_data: .float -1.36
 .float -1.36
 .float 0.0
 .word 0

                                                                                                        
 test_5_data: .float -1.37
 .float -1.36
 .float 0.0
 .word 0

 test_6_data: .float -1.37
 .float -1.36
 .float 0.0
 .word 1

 test_7_data: .float -1.37
 .float -1.36
 .float 0.0
 .word 1

                                                                                                        
                                                                                                        
 test_8_data: .float NaN
 .float 0
 .float 0.0
 .word 0

 test_9_data: .float NaN
 .float NaN
 .float 0.0
 .word 0

 test_10_data: .word 0x7f800001
 .float 0
 .float 0.0
 .word 0

												    
												    
 test_11_data: .float NaN
 .float 0
 .float 0.0
 .word 0

 test_12_data: .float NaN
 .float NaN
 .float 0.0
 .word 0

 test_13_data: .word 0x7f800001
 .float 0
 .float 0.0
 .word 0

 test_14_data: .float NaN
 .float 0
 .float 0.0
 .word 0

 test_15_data: .float NaN
 .float NaN
 .float 0.0
 .word 0

 test_16_data: .word 0x7f800001
 .float 0
 .float 0.0
 .word 0

