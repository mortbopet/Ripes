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
 fdiv.s f3, f0, f1
 fmv.x.s a0, f3
 fsflags a1, x0
 li a2, 1
 bne a0, a3, fail
 bne a1, a2, fail
 
  test_3: li gp, 3
 la a0, test_3_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fdiv.s f3, f0, f1
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
 fdiv.s f3, f0, f1
 fmv.x.s a0, f3
 fsflags a1, x0
 li a2, 0
 bne a0, a3, fail
 bne a1, a2, fail
 
  test_5: li gp, 5
 la a0, test_5_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fsqrt.s f3, f0
 fmv.x.s a0, f3
 fsflags a1, x0
 li a2, 1
 bne a0, a3, fail
 bne a1, a2, fail
    
  test_6: li gp, 6
 la a0, test_6_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fsqrt.s f3, f0
 fmv.x.s a0, f3
 fsflags a1, x0
 li a2, 0
 bne a0, a3, fail
 bne a1, a2, fail
    
  test_7: li gp, 7
 la a0, test_7_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fsqrt.s f3, f0
 fmv.x.s a0, f3
 fsflags a1, x0
 li a2, 0x10
 bne a0, a3, fail
 bne a1, a2, fail
 
  test_8: li gp, 8
 la a0, test_8_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fsqrt.s f3, f0
 fmv.x.s a0, f3
 fsflags a1, x0
 li a2, 1
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

test_2_data: .float 3.14159265
 .float 2.71828182
 .float 0.0
 .float 1.1557273520668288
 
test_3_data: .float -1234
 .float 1235.1
 .float 0.0
 .float -0.9991093838555584
 
test_4_data: .float 3.14159265
 .float 1.0
 .float 0.0
 .float 3.14159265
                                          
test_5_data: .float 3.14159265
 .float 0.0
 .float 0.0
 .float 1.7724538498928541
 
test_6_data: .float 10000
 .float 0.0
 .float 0.0
 .float 100
                                  
test_7_data: .float -1.0
 .float 0.0
 .float 0.0
 .word 0x7FC00000
                                    
test_8_data: .float 171.0
 .float 0.0
 .float 0.0
 .float 13.076696
 
