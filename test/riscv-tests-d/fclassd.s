.text


  #-------------------------------------------------------------
  # Arithmetic tests
  #-------------------------------------------------------------

  test_2: la a0, test_2_data 
 fld fa0, 0(a0)
 fclass.d a0, fa0
 li x29, 1
 li gp, 2
 bne a0, x29, fail
  .data
 .align 3
 test_2_data: .word 0, 0xfff00000
 .text
  test_3: la a0, test_3_data 
 fld fa0, 0(a0)
 fclass.d a0, fa0
 li x29, 2
 li gp, 3
 bne a0, x29, fail
  .data
 .align 3
 test_3_data: .word 0, 0xbff00000
 .text
  test_4: la a0, test_4_data 
 fld fa0, 0(a0)
 fclass.d a0, fa0
 li x29, 4
 li gp, 4
 bne a0, x29, fail
  .data
 .align 3
 test_4_data: .word -1, 0x800fffff
 .text
  test_5: la a0, test_5_data 
 fld fa0, 0(a0)
 fclass.d a0, fa0
 li x29, 8
 li gp, 5
 bne a0, x29, fail
  .data
 .align 3
 test_5_data: .word 0, 0x80000000
 .text
  test_6: la a0, test_6_data 
 fld fa0, 0(a0)
 fclass.d a0, fa0
 li x29, 16
 li gp, 6
 bne a0, x29, fail
  .data
 .align 3
 test_6_data: .word 0,0
 .text
  test_7: la a0, test_7_data 
 fld fa0, 0(a0)
 fclass.d a0, fa0
 li x29, 32
 li gp, 7
 bne a0, x29, fail
  .data
 .align 3
 test_7_data: .word -1, 0x000fffff
 .text
  test_8: la a0, test_8_data 
 fld fa0, 0(a0)
 fclass.d a0, fa0
 li x29, 64
 li gp, 8
 bne a0, x29, fail
  .data
 .align 3
 test_8_data: .word 0, 0x3ff00000
 .text
  test_9: la a0, test_9_data 
 fld fa0, 0(a0)
 fclass.d a0, fa0
 li x29, 128
 li gp, 9
 bne a0, x29, fail
  .data
 .align 3
 test_9_data: .word 0, 0x7ff00000
 .text
  test_10: la a0, test_10_data 
 fld fa0, 0(a0)
 fclass.d a0, fa0
 li x29, 256
 li gp, 10
 bne a0, x29, fail
  .data
 .align 3
 test_10_data: .word 1, 0x7ff00000
 .text
  test_11: la a0, test_11_data 
 fld fa0, 0(a0)
 fclass.d a0, fa0
 li x29, 512
 li gp, 11
 bne a0, x29, fail
  .data
 .align 3
 test_11_data: .word 0, 0x7ff80000
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
	
