.data
argument: .word 7 # Number to find the factorial value of

.text
main:
        lw       a0, argument
        jal      ra, fact
        jal      zero, end

fact:
        addi     sp, sp, -16 
        sw       ra, 8(sp) 
        sw       a0, 0(sp)  
        addi     t0, a0, -1
        bge      t0, zero, nfact

        addi     a0, zero, 1
        addi     sp, sp, 16 
        jalr     x0, x1, 0
                                                     
nfact:
        addi     a0, a0, -1 
        jal      ra, fact 

        addi     t1, a0, 0 
        lw       a0, 0(sp) 
        lw       ra, 8(sp) 
        addi     sp, sp, 16 

        mul      a0, a0, t1
        jalr     x0, x1, 0 
end: