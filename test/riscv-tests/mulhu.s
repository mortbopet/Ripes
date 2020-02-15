.text
 main:


  #-------------------------------------------------------------
  # Arithmetic tests
  #-------------------------------------------------------------

  test_2:
 li x1, 0x00000000
 li x2, 0x00000000
 mulhu x30, x1, x2
 li x29, 0x00000000
 li gp, 2
 bne x30, x29, fail


  test_3:
 li x1, 0x00000001
 li x2, 0x00000001
 mulhu x30, x1, x2
 li x29, 0x00000000
 li gp, 3
 bne x30, x29, fail


  test_4:
 li x1, 0x00000003
 li x2, 0x00000007
 mulhu x30, x1, x2
 li x29, 0x00000000
 li gp, 4
 bne x30, x29, fail



  test_5:
 li x1, 0x00000000
 li x2, 0xffff8000
 mulhu x30, x1, x2
 li x29, 0x00000000
 li gp, 5
 bne x30, x29, fail


  test_6:
 li x1, 0x80000000
 li x2, 0x00000000
 mulhu x30, x1, x2
 li x29, 0x00000000
 li gp, 6
 bne x30, x29, fail


  test_7:
 li x1, 0x80000000
 li x2, 0xffff8000
 mulhu x30, x1, x2
 li x29, 0x7fffc000
 li gp, 7
 bne x30, x29, fail



  test_30:
 li x1, 0xaaaaaaab
 li x2, 0x0002fe7d
 mulhu x30, x1, x2
 li x29, 0x0001fefe
 li gp, 30
 bne x30, x29, fail


  test_31:
 li x1, 0x0002fe7d
 li x2, 0xaaaaaaab
 mulhu x30, x1, x2
 li x29, 0x0001fefe
 li gp, 31
 bne x30, x29, fail



  test_32:
 li x1, 0xff000000
 li x2, 0xff000000
 mulhu x30, x1, x2
 li x29, 0xfe010000
 li gp, 32
 bne x30, x29, fail



  test_33:
 li x1, 0xffffffff
 li x2, 0xffffffff
 mulhu x30, x1, x2
 li x29, 0xfffffffe
 li gp, 33
 bne x30, x29, fail


  test_34:
 li x1, 0xffffffff
 li x2, 0x00000001
 mulhu x30, x1, x2
 li x29, 0x00000000
 li gp, 34
 bne x30, x29, fail


  test_35:
 li x1, 0x00000001
 li x2, 0xffffffff
 mulhu x30, x1, x2
 li x29, 0x00000000
 li gp, 35
 bne x30, x29, fail



  #-------------------------------------------------------------
  # Source/Destination tests
  #-------------------------------------------------------------

  test_8:
 li x1, 13631488
 li x2, 11534336
 mulhu x1, x1, x2
 li x29, 36608
 li gp, 8
 bne x1, x29, fail


  test_9:
 li x1, 14680064
 li x2, 11534336
 mulhu x2, x1, x2
 li x29, 39424
 li gp, 9
 bne x2, x29, fail


  test_10:
 li x1, 13631488
 mulhu x1, x1, x1
 li x29, 43264
 li gp, 10
 bne x1, x29, fail



  bne x0, gp, pass
 fail: li a0, 0
 li a7, 93
 ecall

 pass: li a0, 42
 li a7, 93
 ecall

