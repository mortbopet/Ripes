.text
main:
  #-------------------------------------------------------------
  # Assembler expression tests: operator precedence and
  # left-to-right evaluation of +, -, *, /, %, &, |
  #-------------------------------------------------------------

  # Constants, evaluated when the .equ directive is assembled
.equ MUL_ADD, 2*3+4
.equ SUB_SUB, 10-2-3
.equ DIV_MUL, 100/10*2
.equ NEG_ADD, -5+3
.equ AND_OR, 6&3|8
.equ MIXED, 1+2*3-4/2
.equ PARENS, (2*(3+4)-5%3)
.equ SIZE, 4*8+4
.equ DERIVED, SIZE/4*3+1

test_2:
 li x30, MUL_ADD
 li x29, 10
 li gp, 2
 bne x30, x29, fail

test_3:
 li x30, SUB_SUB
 li x29, 5
 li gp, 3
 bne x30, x29, fail

test_4:
 li x30, DIV_MUL
 li x29, 20
 li gp, 4
 bne x30, x29, fail

test_5:
 li x30, NEG_ADD
 li x29, -2
 li gp, 5
 bne x30, x29, fail

test_6:
 li x30, AND_OR
 li x29, 10
 li gp, 6
 bne x30, x29, fail

test_7:
 li x30, MIXED
 li x29, 5
 li gp, 7
 bne x30, x29, fail

test_8:
 li x30, PARENS
 li x29, 12
 li gp, 8
 bne x30, x29, fail

test_9:
 li x30, DERIVED
 li x29, 28
 li gp, 9
 bne x30, x29, fail

  # Instruction immediates
test_10:
 addi x30, x0, 2*3-1
 li x29, 5
 li gp, 10
 bne x30, x29, fail

test_11:
 addi x30, x0, 20/2-3
 li x29, 7
 li gp, 11
 bne x30, x29, fail

test_12:
 addi x30, x0, 3-2+1
 li x29, 2
 li gp, 12
 bne x30, x29, fail

test_13:
 addi x30, x0, SIZE*2-SIZE/4
 li x29, 63
 li gp, 13
 bne x30, x29, fail

test_14:
 li x1, 1
 slli x30, x1, 8/4/2
 li x29, 2
 li gp, 14
 bne x30, x29, fail

  # Data directives
test_15:
 la x1, tdat
 lw x30, 0(x1)
 li x29, 10
 li gp, 15
 bne x30, x29, fail

test_16:
 la x1, tdat
 lw x30, 4(x1)
 li x29, 5
 li gp, 16
 bne x30, x29, fail

test_17:
 la x1, tdat
 lw x30, 8(x1)
 li x29, -2
 li gp, 17
 bne x30, x29, fail

test_18:
 la x1, tdat
 lw x30, 12(x1)
 li x29, 27
 li gp, 18
 bne x30, x29, fail

  # Memory operand offsets
test_19:
 la x1, tdat
 lw x30, 4*2-4(x1)
 li x29, 5
 li gp, 19
 bne x30, x29, fail

pass:
	li a0, 42
	li a7, 93
	ecall
fail:
	li a0, 0
	li a7, 93
	ecall

.data
tdat:
 .word 2*3+4
 .word 10-2-3
 .word -5+3
 .word 1+2*3+4*5
