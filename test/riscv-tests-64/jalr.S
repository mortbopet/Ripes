.text
main:
  #-------------------------------------------------------------
  # jalr tests. The target address is (rs1 + imm) with the least
  # significant bit cleared.
  #-------------------------------------------------------------

test_2:
 li gp, 2
 la t0, target_2
 jalr t1, t0, 0
link_2:
 j fail
target_2:
 la t2, link_2
 bne t1, t2, fail

test_3:
 li gp, 3
 la t0, target_3
 addi t0, t0, 1
 jalr t1, t0, 0
link_3:
 j fail
target_3:
 la t2, link_3
 bne t1, t2, fail

test_4:
 li gp, 4
 la t0, target_4
 jalr t1, t0, 1
link_4:
 j fail
target_4:
 la t2, link_4
 bne t1, t2, fail

test_5:
 li gp, 5
 la t0, target_5
 addi t0, t0, 5
 jalr t1, t0, -4
link_5:
 j fail
target_5:
 la t2, link_5
 bne t1, t2, fail

test_6:
 li gp, 6
 la t0, target_6
 jalr t0, t0, 1
link_6:
 j fail
target_6:
 la t2, link_6
 bne t0, t2, fail

pass:
 li a0, 42
 li a7, 93
 ecall

fail:
 li a0, 0
 li a7, 93
 ecall