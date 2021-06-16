.text
 .globl _start
 _start: nop

  #-------------------------------------------------------------
  # Basic tests
  #-------------------------------------------------------------

  test_2: lui x1, 0x00000
 li x7, 0x0000000000000000
 li gp, 2
 bne x1, x7, fail

  test_3: lui x1, 0xfffff
srai x1,x1,1
 li x7, 0xfffffffffffff800
 li gp, 3
 bne x1, x7, fail

  test_4: lui x1, 0x7ffff
srai x1,x1,20
 li x7, 0x00000000000007ff
 li gp, 4
 bne x1, x7, fail

  test_5: lui x1, 0x80000
srai x1,x1,20
 li x7, 0xfffffffffffff800
 li gp, 5
 bne x1, x7, fail


  test_6: lui x0, 0x80000
 li x7, 0
 li gp, 6
 bne x0, x7, fail


  bne x0, gp, pass
 fail: li a0, 0
 li a7, 93
 ecall
 pass: li a0, 42
 li a7, 93
 ecall



  .data
 .data 
 .align 4
 .global begin_signature
 begin_signature:

 

.align 4
 .global end_signature
 end_signature:
