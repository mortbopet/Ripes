.text
 main:


  #-------------------------------------------------------------
  # Arithmetic tests
  #-------------------------------------------------------------

  test_2:
 li x1, 0x00000001
 li x2, 0
 sll x30, x1, x2
 li x29, 0x00000001
 li gp, 2
 bne x30, x29, fail


  test_3:
 li x1, 0x00000001
 li x2, 1
 sll x30, x1, x2
 li x29, 0x00000002
 li gp, 3
 bne x30, x29, fail


  test_4:
 li x1, 0x00000001
 li x2, 7
 sll x30, x1, x2
 li x29, 0x00000080
 li gp, 4
 bne x30, x29, fail


  test_5:
 li x1, 0x00000001
 li x2, 14
 sll x30, x1, x2
 li x29, 0x00004000
 li gp, 5
 bne x30, x29, fail


  test_6:
 li x1, 0x00000001
 li x2, 31
 sll x30, x1, x2
 li x29, 0x80000000
 li gp, 6
 bne x30, x29, fail



  test_7:
 li x1, 0xffffffff
 li x2, 0
 sll x30, x1, x2
 li x29, 0xffffffff
 li gp, 7
 bne x30, x29, fail


  test_8:
 li x1, 0xffffffff
 li x2, 1
 sll x30, x1, x2
 li x29, 0xfffffffe
 li gp, 8
 bne x30, x29, fail


  test_9:
 li x1, 0xffffffff
 li x2, 7
 sll x30, x1, x2
 li x29, 0xffffff80
 li gp, 9
 bne x30, x29, fail


  test_10:
 li x1, 0xffffffff
 li x2, 14
 sll x30, x1, x2
 li x29, 0xffffc000
 li gp, 10
 bne x30, x29, fail


  test_11:
 li x1, 0xffffffff
 li x2, 31
 sll x30, x1, x2
 li x29, 0x80000000
 li gp, 11
 bne x30, x29, fail



  test_12:
 li x1, 0x21212121
 li x2, 0
 sll x30, x1, x2
 li x29, 0x21212121
 li gp, 12
 bne x30, x29, fail


  test_13:
 li x1, 0x21212121
 li x2, 1
 sll x30, x1, x2
 li x29, 0x42424242
 li gp, 13
 bne x30, x29, fail


  test_14:
 li x1, 0x21212121
 li x2, 7
 sll x30, x1, x2
 li x29, 0x90909080
 li gp, 14
 bne x30, x29, fail


  test_15:
 li x1, 0x21212121
 li x2, 14
 sll x30, x1, x2
 li x29, 0x48484000
 li gp, 15
 bne x30, x29, fail


  test_16:
 li x1, 0x21212121
 li x2, 31
 sll x30, x1, x2
 li x29, 0x80000000
 li gp, 16
 bne x30, x29, fail



  # Verify that shifts only use bottom six bits

  test_17:
 li x1, 0x21212121
 li x2, 0xffffffc0
 sll x30, x1, x2
 li x29, 0x21212121
 li gp, 17
 bne x30, x29, fail


  test_18:
 li x1, 0x21212121
 li x2, 0xffffffc1
 sll x30, x1, x2
 li x29, 0x42424242
 li gp, 18
 bne x30, x29, fail


  test_19:
 li x1, 0x21212121
 li x2, 0xffffffc7
 sll x30, x1, x2
 li x29, 0x90909080
 li gp, 19
 bne x30, x29, fail


  test_20:
 li x1, 0x21212121
 li x2, 0xffffffce
 sll x30, x1, x2
 li x29, 0x48484000
 li gp, 20
 bne x30, x29, fail


  
  #-------------------------------------------------------------
  # Source/Destination tests
  #-------------------------------------------------------------

  test_22:
 li x1, 0x00000001
 li x2, 7
 sll x1, x1, x2
 li x29, 0x00000080
 li gp, 22
 bne x1, x29, fail


  test_23:
 li x1, 0x00000001
 li x2, 14
 sll x2, x1, x2
 li x29, 0x00004000
 li gp, 23
 bne x2, x29, fail


  test_24:
 li x1, 3
 sll x1, x1, x1
 li x29, 24
 li gp, 24
 bne x1, x29, fail



  bne x0, gp, pass
 fail: li a0, 0
 li a7, 93
 ecall

 pass: li a0, 42
 li a7, 93
 ecall


