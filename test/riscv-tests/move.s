.text
 main:


  test_2:
 csrwi fcsr, 1
 li a0, 0x1234
 fssr a1, a0
 li x29, 1
 li gp, 2
 bne a1, x29, fail

  test_3:
 frsr a0
 li x29, 0x34
 li gp, 3
 bne a0, x29, fail

  test_4:
 frflags a0
 li x29, 0x14
 li gp, 4
 bne a0, x29, fail

  test_5:
 csrrwi a0, frm, 2
 li x29, 0x01
 li gp, 5
 bne a0, x29, fail

  test_6:
 frsr a0
 li x29, 0x54
 li gp, 6
 bne a0, x29, fail

  test_7:
 csrrci a0, fflags, 4
 li x29, 0x14
 li gp, 7
 bne a0, x29, fail

  test_8:
 frsr a0
 li x29, 0x50
 li gp, 8
 bne a0, x29, fail

# 34 "rv32uf/../rv64uf/move.S"
 
  test_10: li a1, 0x12345678
 li a2, 0
 fmv.s.x f1, a1
 fmv.s.x f2, a2
 fsgnj.s f0, f1, f2
 fmv.x.s a0, f0
 li x29, 0x12345678
 li gp, 10
 bne a0, x29, fail

  test_11: li a1, 0x12345678
 li a2, -1
 fmv.s.x f1, a1
 fmv.s.x f2, a2
 fsgnj.s f0, f1, f2
 fmv.x.s a0, f0
 li x29, 0x92345678
 li gp, 11
 bne a0, x29, fail

  test_12: li a1, 0x92345678
 li a2, 0
 fmv.s.x f1, a1
 fmv.s.x f2, a2
 fsgnj.s f0, f1, f2
 fmv.x.s a0, f0
 li x29, 0x12345678
 li gp, 12
 bne a0, x29, fail

  test_13: li a1, 0x92345678
 li a2, -1
 fmv.s.x f1, a1
 fmv.s.x f2, a2
 fsgnj.s f0, f1, f2
 fmv.x.s a0, f0
 li x29, 0x92345678
 li gp, 13
 bne a0, x29, fail


  test_20: li a1, 0x12345678
 li a2, 0
 fmv.s.x f1, a1
 fmv.s.x f2, a2
 fsgnjn.s f0, f1, f2
 fmv.x.s a0, f0
 li x29, 0x92345678
 li gp, 20
 bne a0, x29, fail

  test_21: li a1, 0x12345678
 li a2, -1
 fmv.s.x f1, a1
 fmv.s.x f2, a2
 fsgnjn.s f0, f1, f2
 fmv.x.s a0, f0
 li x29, 0x12345678
 li gp, 21
 bne a0, x29, fail

  test_22: li a1, 0x92345678
 li a2, 0
 fmv.s.x f1, a1
 fmv.s.x f2, a2
 fsgnjn.s f0, f1, f2
 fmv.x.s a0, f0
 li x29, 0x92345678
 li gp, 22
 bne a0, x29, fail

  test_23: li a1, 0x92345678
 li a2, -1
 fmv.s.x f1, a1
 fmv.s.x f2, a2
 fsgnjn.s f0, f1, f2
 fmv.x.s a0, f0
 li x29, 0x12345678
 li gp, 23
 bne a0, x29, fail


  test_30: li a1, 0x12345678
 li a2, 0
 fmv.s.x f1, a1
 fmv.s.x f2, a2
 fsgnjx.s f0, f1, f2
 fmv.x.s a0, f0
 li x29, 0x12345678
 li gp, 30
 bne a0, x29, fail

  test_31: li a1, 0x12345678
 li a2, -1
 fmv.s.x f1, a1
 fmv.s.x f2, a2
 fsgnjx.s f0, f1, f2
 fmv.x.s a0, f0
 li x29, 0x92345678
 li gp, 31
 bne a0, x29, fail

  test_32: li a1, 0x92345678
 li a2, 0
 fmv.s.x f1, a1
 fmv.s.x f2, a2
 fsgnjx.s f0, f1, f2
 fmv.x.s a0, f0
 li x29, 0x92345678
 li gp, 32
 bne a0, x29, fail

  test_33: li a1, 0x92345678
 li a2, -1
 fmv.s.x f1, a1
 fmv.s.x f2, a2
 fsgnjx.s f0, f1, f2
 fmv.x.s a0, f0
 li x29, 0x12345678
 li gp, 33
 bne a0, x29, fail

 bne x0, gp, pass
 fail: li a0, 0
 li a7, 93
 ecall

 pass: li a0, 42
 li a7, 93
 ecall

