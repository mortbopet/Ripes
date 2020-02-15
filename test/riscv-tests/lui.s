.text
main:
  #-------------------------------------------------------------
  # Basic tests
  #-------------------------------------------------------------

test_2:
 lui x1, 0x00000
 li x29, 0x00000000
 li gp, 2
 bne x1, x29, fail


test_3:
 lui x1, 0xfffff
 srai x1,x1,1
 li x29, 0xfffff800
 li gp, 3
 bne x1, x29, fail


test_4:
 lui x1, 0x7ffff
 srai x1,x1,20
 li x29, 0x000007ff
 li gp, 4
 bne x1, x29, fail


test_5:
 lui x1, 0x80000
 srai x1,x1,20
 li x29, 0xfffff800
 li gp, 5
 bne x1, x29, fail



test_6:
 lui x0, 0x80000
 li x29, 0
 li gp, 6
 bne x0, x29, fail

pass:
 li a0, 42
 li a7, 93
 ecall

fail:
 li a0, 0
 li a7, 93
 ecall



