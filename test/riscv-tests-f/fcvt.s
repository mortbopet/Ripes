.text
 main:
 li s0, 0x00000000ffffffff # single float mask for 64 bit processors
 j test_2

  setup:
 lw a3, 0(a0)
 and a3, a3, s0 # remove sign extended high word for 64 bit processors
 ret



  #-------------------------------------------------------------
  # Arithmetic tests
  #-------------------------------------------------------------

  test_2: li gp, 2
 la a0, test_2_data 
 jal setup
 li a0, 2
 fcvt.s.w f0, a0
 fsflags x0, x0
 fmv.x.s a0, f0
 bne a0, a3, fail
   
  test_3: li gp, 3
 la a0, test_3_data 
 jal setup
 li a0, -2
 fcvt.s.w f0, a0
 fsflags x0, x0
 fmv.x.s a0, f0
 bne a0, a3, fail
  
  test_4: li gp, 4
 la a0, test_4_data 
 jal setup
 li a0, 2
 fcvt.s.wu f0, a0
 fsflags x0, x0
 fmv.x.s a0, f0
 bne a0, a3, fail
  
  test_5: li gp, 5
 la a0, test_5_data 
 jal setup
 li a0, -2
 fcvt.s.wu f0, a0
 fsflags x0, x0
 fmv.x.s a0, f0
 bne a0, a3, fail
 
  bne x0, gp, pass
 fail: li a0, 0
 li a7, 93
 ecall

 pass: li a0, 42
 li a7, 93
 ecall


.data
test_2_data: .float 2.0

test_3_data: .float -2.0

test_4_data: .float 2.0

test_5_data: .float 4.2949673e9

