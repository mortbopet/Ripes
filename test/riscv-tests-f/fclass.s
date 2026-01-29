.text
 main:


  #-------------------------------------------------------------
  # Arithmetic tests
  #-------------------------------------------------------------

  test_2:
 li a0, 0xff800000
 fmv.s.x fa0, a0
 fclass.s a0, fa0
 li x29, 1
 li gp, 2
 bne a0, x29, fail

  test_3:
 li a0, 0xbf800000
 fmv.s.x fa0, a0
 fclass.s a0, fa0
 li x29, 2
 li gp, 3
 bne a0, x29, fail

  test_4:
 li a0, 0x807fffff
 fmv.s.x fa0, a0
 fclass.s a0, fa0
 li x29, 4
 li gp, 4
 bne a0, x29, fail

  test_5:
 li a0, 0x80000000
 fmv.s.x fa0, a0
 fclass.s a0, fa0
 li x29, 8
 li gp, 5
 bne a0, x29, fail

  test_6:
 li a0, 0x00000000
 fmv.s.x fa0, a0
 fclass.s a0, fa0
 li x29, 0x10
 li gp, 6
 bne a0, x29, fail

  test_7:
 li a0, 0x007fffff
 fmv.s.x fa0, a0
 fclass.s a0, fa0
 li x29, 0x20
 li gp, 7
 bne a0, x29, fail

  test_8:
 li a0, 0x3f800000
 fmv.s.x fa0, a0
 fclass.s a0, fa0
 li x29, 0x40
 li gp, 8
 bne a0, x29, fail

  test_9:
 li a0, 0x7f800000
 fmv.s.x fa0, a0
 fclass.s a0, fa0
 li x29, 0x80
 li gp, 9
 bne a0, x29, fail

  test_10:
 li a0, 0x7f800001
 fmv.s.x fa0, a0
 fclass.s a0, fa0
 li x29,0x100
 li gp, 10
 bne a0, x29, fail

  test_11:
 li a0, 0x7fc00000
 fmv.s.x fa0, a0
 fclass.s a0, fa0
 li x29,0x200
 li gp, 11
 bne a0, x29, fail


  bne x0, gp, pass
 fail: li a0, 0
 li a7, 93
 ecall

 pass: li a0, 42
 li a7, 93
 ecall

