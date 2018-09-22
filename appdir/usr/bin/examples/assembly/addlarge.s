	.text
	li	a0,-2147483647
	li	a1,2147483646
	add	a2,a0,a1
	addi 	a0,x0,10
	ecall
