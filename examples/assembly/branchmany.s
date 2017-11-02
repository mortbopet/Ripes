	.text
	li a0,10
	li a1,20
	beq a0,a1,e
	blt a0,a1,b
a:
	li a0,3
	li a1,4
	blt a0,a1,f
b:
	blt a0,a1,d
c:
	bgt a1,a0,a
d:
	blt a0,a1,c
e:
	li a0,1
	li a1,2
f:
	mv	a2,a0
	li	a0,10
	ecall
