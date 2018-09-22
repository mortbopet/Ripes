	.text
main:
	li a0,0
	li a1,10
loop:
	addi a0,a0,1
	bne a0,a1,loop
	blt a0,a1,main
	bgt a0,a1,main
	mv a2,a0
	li a0,10
	ecall
