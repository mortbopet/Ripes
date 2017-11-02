	.text
	li a0,0xff000000
	li a1,0x000000ff
	srai a0,a0,8
	slli a1,a1,8
	addi a1,a1,0xff
	add	a2,a0,a1
	addi a0,x0,10
	ecall
