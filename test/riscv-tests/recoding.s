.text
 main:


  # Make sure infinities with different mantissas compare as equal.
 flw f0, minf, a0
 flw f1, three, a0
 fmul.s f1, f1, f0
  test_2:
 feq.s a0, f0, f1
 li x29, 1
 li gp, 2
 bne a0, x29, fail

  test_3:
 fle.s a0, f0, f1
 li x29, 1
 li gp, 3
 bne a0, x29, fail

  test_4:
 flt.s a0, f0, f1
 li x29, 0
 li gp, 4
 bne a0, x29, fail


  # Likewise, but for zeroes.
  fcvt.s.w f0, x0
  li a0, 1
  fcvt.s.w f1, a0
  fmul.s f1, f1, f0
  test_5:
 feq.s a0, f0, f1
 li x29, 1
 li gp, 5
 bne a0, x29, fail

  test_6:
 fle.s a0, f0, f1
 li x29, 1
 li gp, 6
 bne a0, x29, fail

  test_7:
 flt.s a0, f0, f1
 li x29, 0
 li gp, 7
 bne a0, x29, fail


  bne x0, gp, pass
 fail: li a0, 0
 li a7, 93
 ecall

 pass: li a0, 42
 li a7, 93
 ecall


.data
minf: .float -Inf
three: .float 3.0
