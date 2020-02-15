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
 fadd.s f3, f0, f1
 fmv.x.s a0, f3
 fsflags a1, x0
 li a2, 0
 bne a0, a3, fail
 bne a1, a2, fail
    
  test_3: li gp, 3
 la a0, test_3_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fadd.s f3, f0, f1
 fmv.x.s a0, f3
 fsflags a1, x0
 li a2, 1
 bne a0, a3, fail
 bne a1, a2, fail
    
  test_4: li gp, 4
 la a0, test_4_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fadd.s f3, f0, f1
 fmv.x.s a0, f3
 fsflags a1, x0
 li a2, 1
 bne a0, a3, fail
 bne a1, a2, fail
    

  test_5: li gp, 5
 la a0, test_5_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fsub.s f3, f0, f1
 fmv.x.s a0, f3
 fsflags a1, x0
 li a2, 0
 bne a0, a3, fail
 bne a1, a2, fail
    
  test_6: li gp, 6
 la a0, test_6_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fsub.s f3, f0, f1
 fmv.x.s a0, f3
 fsflags a1, x0
 li a2, 1
 bne a0, a3, fail
 bne a1, a2, fail
    
  test_7: li gp, 7
 la a0, test_7_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fsub.s f3, f0, f1
 fmv.x.s a0, f3
 fsflags a1, x0
 li a2, 1
 bne a0, a3, fail
 bne a1, a2, fail
    

  test_8: li gp, 8
 la a0, test_8_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fmul.s f3, f0, f1
 fmv.x.s a0, f3
 fsflags a1, x0
 li a2, 0
 bne a0, a3, fail
 bne a1, a2, fail
    
  test_9: li gp, 9
 la a0, test_9_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fmul.s f3, f0, f1
 fmv.x.s a0, f3
 fsflags a1, x0
 li a2, 1
 bne a0, a3, fail
 bne a1, a2, fail
    
  test_10: li gp, 10
 la a0, test_10_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fmul.s f3, f0, f1
 fmv.x.s a0, f3
 fsflags a1, x0
 li a2, 1
 bne a0, a3, fail
 bne a1, a2, fail
 

  # Is the canonical NaN generated for Inf - Inf?
  test_11: li gp, 11
 la a0, test_11_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fsub.s f3, f0, f1
 fmv.x.s a0, f3
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

 test_2_data: .float 2.5
 .float 1.0
 .float 0.0
 .float 3.5
 
 test_3_data: .float -1235.1
 .float 1.1
 .float 0.0
 .float -1234
 
 test_4_data: .float 3.14159265
 .float 0.00000001
 .float 0.0
 .float 3.14159265
 
                                                                                                                                 
 test_5_data: .float 2.5
 .float 1.0
 .float 0.0
 .float 1.5
 
 test_6_data: .float -1235.1
 .float -1.1
 .float 0.0
 .float -1234
 
 test_7_data: .float 3.14159265
 .float 0.00000001
 .float 0.0
 .float 3.14159265
 
                                                                                                                                 
 test_8_data: .float 2.5
 .float 1.0
 .float 0.0
 .float 2.5
 
 test_9_data: .float -1235.1
 .float -1.1
 .float 0.0
 .float 1358.61
 
 test_10_data: .float 3.14159265
 .float 0.00000001
 .float 0.0
 .float 3.14159265e-8
 
 test_11_data: .float Inf
 .float Inf
 .float 0.0
 .word 0x7fc00000
 
