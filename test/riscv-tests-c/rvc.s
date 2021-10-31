.text
main:
 jal zero, test_2
 
#data
d1: .word 0x76543210
d2: .word 0xfedcba98
d3: .word 0x76543210
d4: .word 0xfedcba98

test_2:
 addi a1,a1,1
 li t2,1
 li gp,2
 bne a1,t2,fail
 lui sp,0x1
 addi sp,sp,564 
    
test_3:
 c.addi4spn a0,1020
 lui t2,0x1
 addi t2,t2,1584 
 li gp,3
 bne a0,t2, fail 
    
test_4:
 c.addi16sp 496
 c.nop 
 lui t2,0x1
 addi t2,t2,1060 
 li gp,4
 bne sp,t2,fail    
   
    
test_5:
 c.addi16sp -512
 c.nop
 lui t2,0x1
 addi t2,t2,548
 li gp,5
 bne sp,t2, fail
 li a1,0
 addi a1,a1, d1         
 
test_6:
 c.lw a0, a1,4  #c.lw a0, 4(a1) better format ?
 c.addi  a0, 1
 c.sw a0, a1, 4 #c.sw a0, 4(a1)
 c.lw a2, a1 ,4 #c.lw a2, 4(a1)
 lui t2,0xfedcc
 addi t2,t2,-1383 
 li gp,6
 bne a2,t2, fail

test_8:
 ori a0,zero,1
 c.addi a0,-16
 c.nop
 li t2,-15
 li gp,8
 bne a0,t2, fail 
 
test_9:
 ori a5,zero,1
 c.li a5,-16
 c.nop
 li t2,-16
 li gp,9
 bne a5,t2, fail

test_11:
 c.lui s0, -126976
 c.srai s0,12
 li t2,-31
 li gp,11
 bne s0,t2, fail

test_12:
 c.lui s0, -126976
 c.srli s0,12
 lui t2,0x100
 addi t2,t2,-31 
 li gp,12
 bne s0,t2, fail

test_14:
 c.li s0,-2
 c.andi s0,-17
 li t2,-18
 li gp,14
 bne s0,t2, fail

test_15:
 c.li s1,20
 c.li a0,6
 c.sub s1,a0
 c.nop
 li t2,14
 li gp,15
 bne s1,t2, fail

test_16:
 c.li s1,20
 c.li a0,6
 c.xor s1,a0
 c.nop
 li t2,18
 li gp,16
 bne s1,t2, fail

test_17:
 c.li s1,20
 c.li a0,6
 c.or s1,a0
 c.nop
 li t2,22
 li gp,17
 bne s1,t2, fail

test_18:
 c.li s1,20
 c.li a0,6
 c.and s1,a0
 c.nop
 li t2,4
 li gp,18
 bne s1,t2, fail

test_21:
 lui s0,0x1
 addi s0,s0,564 
 c.slli s0,0x4
 c.nop
 lui t2,0x12
 addi t2,t2,832 
 li gp,21
 bne s0,t2, fail

test_30:
 c.li ra,0
 c.j 4
 c.j 4
 c.j 4
 c.j fail
 c.nop
 li t2,0
 li gp,30
 bne ra,t2, fail

test_31:
 c.li a0,0
 c.beqz a0, 4
 c.j 4
 c.j 4
 c.j fail
 c.nop
 li t2,0
 li gp,31
 bne zero,t2, fail

test_32:
 c.li a0,1
 c.bnez a0,4
 c.j 4
 c.j 4
 c.j fail
 c.nop
 li t2,0
 li gp,32
 bne zero,t2, fail

test_33:
 c.li a0,1
 c.beqz a0,4
 c.j 4
 c.j fail
 li t2,0
 li gp,33
 bne zero,t2,fail

test_34:
 c.li a0,0
 c.bnez a0, 4
 c.j 4
 c.j fail
 li t2,0
 li gp,34
 bne zero,t2,fail

test_35:
 auipc t0,0x0
 addi t0,t0,14 
 c.li ra,0
 c.jr t0
 c.j 4
 c.j 4
 c.j fail
 c.nop
 nop
 li t2,0
 li gp,35
 bne ra,t2,fail

test_36:
 auipc t0,0x0
 addi t0,t0,14 
 c.li ra,0
 c.jalr t0
 c.j 4
 c.j 4
 c.j fail
 sub ra,ra,t0
 c.nop
 li t2,-2
 li gp,36
 bne ra,t2,fail

test_37:
 auipc t0,0x0
 addi t0,t0,14 
 c.li ra,0
 c.jal 4
 c.j 4
 c.j 4
 c.j fail
 sub ra,ra,t0
 c.nop
 li t2,-2
 li gp,37
 bne ra,t2,fail
 li     sp,0
 addi sp,sp, d1 

test_40:
 c.lwsp  a0, 12 
 c.addi a0,1  
 c.swsp  a0, 12
 c.lwsp  a2, 12 
 lui t2,0xfedcc
 addi t2,t2,-1383 
 li gp,40
 bne a2,t2,fail

test_42:
 li a0,291
 c.mv t0,a0
 c.add t0,a0
 li t2,582
 li gp,42
 bne t0,t2, fail 
    
    
 bne zero,gp, pass
  

pass:
 li a0, 42
 li a7, 93
 ecall

fail:
 li a0, 0
 li a7, 93
 ecall
