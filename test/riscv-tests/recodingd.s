.text

  # Make sure infinities with different mantissas compare as equal.
  fld f0, minf, a0
  fld f1, three, a0
  fmul.d f1, f1, f0
  test_2: feq.d a0, f0, f1
 li x29, 1
 li gp, 2
 bne a0, x29, fail

  test_3: fle.d a0, f0, f1
 li x29, 1
 li gp, 3
 bne a0, x29, fail

  test_4: flt.d a0, f0, f1
 li x29, 0
 li gp, 4
 bne a0, x29, fail


  # Likewise, but for zeroes.
  fcvt.d.w f0, x0
  li a0, 1
  fcvt.d.w f1, a0
  fmul.d f1, f1, f0
  test_5: feq.d a0, f0, f1
 li x29, 1
 li gp, 5
 bne a0, x29, fail

  test_6: fle.d a0, f0, f1
 li x29, 1
 li gp, 6
 bne a0, x29, fail

  test_7: flt.d a0, f0, f1
 li x29, 0
 li gp, 7
 bne a0, x29, fail


  # When converting small doubles to single-precision subnormals,
  # ensure that the extra precision is discarded.
  flw f0, big, a0
  fld f1, tiny, a0
  fcvt.s.d f1, f1
  fmul.s f0, f0, f1
  fmv.x.s a0, f0
  lw a1, small
  test_10: sub a0, a0, a1
 li x29, 0
 li gp, 10
 bne a0, x29, fail


  # Make sure FSD+FLD correctly saves and restores a single-precision value.
  flw f0, three, a0
  fadd.s f1, f0, f0
  fadd.s f0, f0, f0
  fsd f0, tiny, a0
  fld f0, tiny, a0
  test_20: feq.s a0, f0, f1
 li x29, 1
 li gp, 20
 bne a0, x29, fail

	j success
fail:
	li a0, 0
	li a7, 93 
	ecall
	
success:
	li a0, 42 
	li a7, 93
	ecall


.data
minf: .double -Inf
three: .double 3.0
big: .float 1221
small: .float 2.9133121e-37
tiny: .double 2.3860049081905093e-40
