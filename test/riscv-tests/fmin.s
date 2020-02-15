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
 fmin.s f3, f0, f1
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
 fmin.s f3, f0, f1
 fmv.x.s a0, f3
 fsflags a1, x0
 li a2, 0
 bne a0, a3, fail
 bne a1, a2, fail
       
  test_4: li gp, 4
 la a0, test_4_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fmin.s f3, f0, f1
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
 fmin.s f3, f0, f1
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
 fmin.s f3, f0, f1
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
 fmin.s f3, f0, f1
 fmv.x.s a0, f3
 fsflags a1, x0
 li a2, 0
 bne a0, a3, fail
 bne a1, a2, fail
       

  test_12: li gp, 12
 la a0, test_12_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fmax.s f3, f0, f1
 fmv.x.s a0, f3
 fsflags a1, x0
 li a2, 0
 bne a0, a3, fail
 bne a1, a2, fail
    
  test_13: li gp, 13
 la a0, test_13_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fmax.s f3, f0, f1
 fmv.x.s a0, f3
 fsflags a1, x0
 li a2, 0
 bne a0, a3, fail
 bne a1, a2, fail
    
  test_14: li gp, 14
 la a0, test_14_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fmax.s f3, f0, f1
 fmv.x.s a0, f3
 fsflags a1, x0
 li a2, 0
 bne a0, a3, fail
 bne a1, a2, fail
    
  test_15: li gp, 15
 la a0, test_15_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fmax.s f3, f0, f1
 fmv.x.s a0, f3
 fsflags a1, x0
 li a2, 0
 bne a0, a3, fail
 bne a1, a2, fail
    
  test_16: li gp, 16
 la a0, test_16_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fmax.s f3, f0, f1
 fmv.x.s a0, f3
 fsflags a1, x0
 li a2, 0
 bne a0, a3, fail
 bne a1, a2, fail
    
  test_17: li gp, 17
 la a0, test_17_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fmax.s f3, f0, f1
 fmv.x.s a0, f3
 fsflags a1, x0
 li a2, 0
 bne a0, a3, fail
 bne a1, a2, fail
    

  # FMIN(0d:7ff0000000000001, x) = x

  test_20: li gp, 20
 la a0, test_20_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fmax.s f3, f0, f1
 fmv.x.s a0, f3
 fsflags a1, x0
 li a2, 0x10
 bne a0, a3, fail
 bne a1, a2, fail
 
  # FMIN(0d:7ff8000000000000, 0d:7ff8000000000000) = canonical NaN

  test_21: li gp, 21
 la a0, test_21_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fmax.s f3, f0, f1
 fmv.x.s a0, f3
 fsflags a1, x0
 li a2, 0x00
 bne a0, a3, fail
 bne a1, a2, fail
 

  # -0.0 < +0.0
  test_30: li gp, 30
 la a0, test_30_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fmin.s f3, f0, f1
 fmv.x.s a0, f3
 fsflags a1, x0
 li a2, 0
 bne a0, a3, fail
 bne a1, a2, fail
    
  test_31: li gp, 31
 la a0, test_31_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fmin.s f3, f0, f1
 fmv.x.s a0, f3
 fsflags a1, x0
 li a2, 0
 bne a0, a3, fail
 bne a1, a2, fail
    
  test_32: li gp, 32
 la a0, test_32_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fmax.s f3, f0, f1
 fmv.x.s a0, f3
 fsflags a1, x0
 li a2, 0
 bne a0, a3, fail
 bne a1, a2, fail
    
  test_33: li gp, 33
 la a0, test_33_data 
 flw f0, 0(a0)
 flw f1, 4(a0)
 flw f2, 8(a0)
 lw a3, 12(a0)
 fmax.s f3, f0, f1
 fmv.x.s a0, f3
 fsflags a1, x0
 li a2, 0
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
 .float 1.0
 
 test_3_data: .float -1235.1
 .float 1.1
 .float 0.0
 .float -1235.1
 
 test_4_data: .float 1.1
 .float -1235.1
 .float 0.0
 .float -1235.1
 
 test_5_data: .float NaN
 .float -1235.1
 .float 0.0
 .float -1235.1
 
 test_6_data: .float 3.14159265
 .float 0.00000001
 .float 0.0
 .float 0.00000001
 
 test_7_data: .float -1.0
 .float -2.0
 .float 0.0
 .float -2.0
 
                                                                                                                             
 test_12_data: .float 2.5
 .float 1.0
 .float 0.0
 .float 2.5
 
 test_13_data: .float -1235.1
 .float 1.1
 .float 0.0
 .float 1.1
 
 test_14_data: .float 1.1
 .float -1235.1
 .float 0.0
 .float 1.1
 
 test_15_data: .float NaN
 .float -1235.1
 .float 0.0
 .float -1235.1
 
 test_16_data: .float 3.14159265
 .float 0.00000001
 .float 0.0
 .float 3.14159265
 
 test_17_data: .float -1.0
 .float -2.0
 .float 0.0
 .float -1.0
 
                                                                                                                             
                                                                                                                             
 test_20_data: .word 0x7f800001
 .float 1.0
 .float 0.0
 .float 1.0
 
 test_21_data: .float NaN
 .float NaN
 .float 0.0
 .word 0x7fc00000
 
                                                                                                                             
                                                                                                                             
 test_30_data: .float -0.0
 .float 0.0
 .float 0.0
 .float -0.0
 
 test_31_data: .float 0.0
 .float -0.0
 .float 0.0
 .float -0.0
 
 test_32_data: .float -0.0
 .float 0.0
 .float 0.0
 .float 0.0
 
 test_33_data: .float 0.0
 .float -0.0
 .float 0.0
 .float 0.0
 
