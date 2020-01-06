.text
main:
  # Buffer
  li s0, 0x1000

  #-------------------------------------------------------------
  # Memory tests
  #-------------------------------------------------------------

# word load/store
test_2:
  jal reset
  li t0, 0x12341234
  sw t0, 0(s0)
  lw t1, 0(s0)
  li gp, 2
  bne t0, t1, fail

# Half-word load/store
test_3:
  jal reset
  li t0, -1234
  sh t0, 0(s0)
  lh t1, 0(s0)
  lhu t2, 0(s0)
  li gp, 3
  bne t0, t1, fail
  li t3, 64302
  bne t2, t3, fail

# Byte load/store
test_4:
  jal reset
  li t0, -123
  sh t0, 0(s0)
  lb t1, 0(s0)
  lbu t2, 0(s0)
  li gp, 4
  bne t0, t1, fail
  li t3, 133
  bne t2, t3, fail

pass:
	li a0, 42
	li a7, 93
	ecall
fail:
	li a0, 0
	li a7, 93
	ecall

reset:
  li t0, 0
  li t1, 0
  li t2, 0
  li t3, 0
  sw zero, 0(s0)
  ret
