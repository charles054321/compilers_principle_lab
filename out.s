.data
_prompt: .asciiz "Enter an integer:"
_ret: .asciiz "\n"
.globl main
.text
read:
  li $v0, 4
  la $a0, _prompt
  syscall
  li $v0, 5
  syscall
  jr $ra

write:
  li $v0, 1
  syscall
  li $v0, 4
  la $a0, _ret
  syscall
  move $v0, $0
  jr $ra

fact:
  beq $t1, 1, label1
  j label2
label1:
  move $v0, $t1
  jr $ra
  j label3
label2:
  addi $t2, $t1, -1
  addi $sp, $sp, -8
  sw $a0, 0($sp)
  sw $ra, 4($sp)
  move $a0, $t2
  jal fact
  move $t3, $v0
  lw $a0, 0($sp)
  lw $ra, 4($sp)
  addi $sp, $sp, 8
  mul $t4, $t1, $t3
  move $v0, $t4
  jr $ra
label3:
  li $t1, -1
  move $v0, $t1
  jr $ra

main:
  addi $sp, $sp, -4
  sw $ra, 0($sp)
  jal read
  lw $ra, 0($sp)
  addi $sp, $sp, 4
  move $t5, $v0
  move $t6, $t5
  bgt $t6, 1, label4
  j label5
label4:
  addi $sp, $sp, -4
  sw $ra, 0($sp)
  move $a0, $t6
  jal fact
  move $t7, $v0
  lw $ra, 0($sp)
  addi $sp, $sp, 8
  move $t0, $t7
  j label6
label5:
  li $t0, 1
label6:
  move $a0, $t0
  addi $sp, $sp, -4
  sw $ra, 0($sp)
  jal write
  lw $ra, 0($sp)
  addi $sp, $sp, 4
  li $t1, 0
  move $v0, $t1
  jr $ra
