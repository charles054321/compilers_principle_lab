#include "common.h"
#include "inter_code.h"
#include <string.h>
#include <stdlib.h>

static const char *MIPS[] = {
    "%s:\n",
    "\n%s:\n",
    " %s, %s\n",
    " %s, %s, %s\n",
    " %s, %s, ",
    "  mul %s, %s, %s\n",
    "  div %s, %s\n  mflo %s\n",
    "",
    "  lw %s, 0(%s)\n",
    "  sw %s, 0(%s)\n",
    "  j %s\n",
    " %s, %s, %s\n",
    "  move $v0, %s\n  jr $ra\n",
    "",
    "",
    "  jal %s\n  move %s, $v0\n",
    "",
    "\
  addi $sp, $sp, -4\n\
  sw $ra, 0($sp)\n\
  jal read\n\
  lw $ra, 0($sp)\n\
  addi $sp, $sp, 4\n\
  move %s, $v0\n",
    "\
  move $a0, %s\n\
  addi $sp, $sp, -4\n\
  sw $ra, 0($sp)\n\
  jal write\n\
  lw $ra, 0($sp)\n\
  addi $sp, $sp, 4\n"
};

















