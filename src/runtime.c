/*
  runtime.c: Sally language runtime support v2003.1104
  (c)2003 Cat's Eye Technologies.  All rights reserved.
*/

#include <stdio.h>

/******************************    RUNTIME    ******************************/

int stack[256];
int sp=0;

int pop(void)
{
  if (sp > 0) return stack[sp--]; else return 0;
}

void push(int i)
{
  if (sp < 255) stack[++sp] = i;
}

void apply_add(void)
{
  int arg2 = pop();
  int arg1 = pop();
  push(arg1 + arg2);
}

void apply_sub(void)
{
  int arg2 = pop();
  int arg1 = pop();
  push(arg1 - arg2);
}

void apply_mul(void)
{
  int arg2 = pop();
  int arg1 = pop();
  push(arg1 * arg2);
}

void apply_div(void)
{
  int arg2 = pop();
  int arg1 = pop();
  push(arg1 / arg2);
}

void apply_dup(void)
{
  int arg1 = pop();
  push(arg1);
  push(arg1);
}

void apply_pop(void)
{
  pop();
}

void apply_print(void)
{
  fprintf(stdout, "%c", (char)pop());
}

void apply_input(void)
{
  push(fgetc(stdin));
}

/* END of runtime.c */
