.text
 .globl _start
 _start: nop

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

  test_2: li x14, MUL_ADD
 li x7, 10
 li gp, 2
 bne x14, x7, fail

  test_3: li x14, SUB_SUB
 li x7, 5
 li gp, 3
 bne x14, x7, fail

  test_4: li x14, DIV_MUL
 li x7, 20
 li gp, 4
 bne x14, x7, fail

  test_5: li x14, NEG_ADD
 li x7, -2
 li gp, 5
 bne x14, x7, fail

  test_6: li x14, AND_OR
 li x7, 10
 li gp, 6
 bne x14, x7, fail

  test_7: li x14, MIXED
 li x7, 5
 li gp, 7
 bne x14, x7, fail

  test_8: li x14, PARENS
 li x7, 12
 li gp, 8
 bne x14, x7, fail

  test_9: li x14, DERIVED
 li x7, 28
 li gp, 9
 bne x14, x7, fail

  # Instruction immediates
  test_10: addi x14, x0, 2*3-1
 li x7, 5
 li gp, 10
 bne x14, x7, fail

  test_11: addi x14, x0, 20/2-3
 li x7, 7
 li gp, 11
 bne x14, x7, fail

  test_12: addi x14, x0, 3-2+1
 li x7, 2
 li gp, 12
 bne x14, x7, fail

  test_13: addi x14, x0, SIZE*2-SIZE/4
 li x7, 63
 li gp, 13
 bne x14, x7, fail

  test_14: li x1, 1
 slli x14, x1, 8/4/2
 li x7, 2
 li gp, 14
 bne x14, x7, fail

  # Data directives
  test_15: la x1, tdat
 lw x14, 0(x1)
 li x7, 10
 li gp, 15
 bne x14, x7, fail

  test_16: la x1, tdat
 lw x14, 4(x1)
 li x7, 5
 li gp, 16
 bne x14, x7, fail

  test_17: la x1, tdat
 lw x14, 8(x1)
 li x7, -2
 li gp, 17
 bne x14, x7, fail

  test_18: la x1, tdat
 lw x14, 12(x1)
 li x7, 27
 li gp, 18
 bne x14, x7, fail

  # Memory operand offsets
  test_19: la x1, tdat
 lw x14, 4*2-4(x1)
 li x7, 5
 li gp, 19
 bne x14, x7, fail

  bne x0, gp, pass
 fail: li a0, 0
 li a7, 93
 ecall
 pass: li a0, 42
 li a7, 93
 ecall



  .data
 .data
 .align 4
 .global begin_signature
 begin_signature:



tdat:
tdat1: .word 2*3+4
tdat2: .word 10-2-3
tdat3: .word -5+3
tdat4: .word 1+2*3+4*5

.align 4
 .global end_signature
 end_signature:
