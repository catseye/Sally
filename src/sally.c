/*
  sally.c: Sally language subroutines v2003.1104
  (c)2003 Cat's Eye Technologies.  All rights reserved.
*/

#include <string.h>
#include <ctype.h>

#include "sally.h"

/******************************  TYPE SYSTEM  ******************************/

type * typestack[256];
int tsp = 0;

type * debit_type(void) /* pop type from type stack */
{
  if (tsp > 0) return typestack[tsp--];
  error("Type underflow");
  return NULL;
}

void credit_type(type * t) /* push type onto type stack */
{
  if (tsp < 255) typestack[++tsp] = t; else error("Type Overflow");
}

type * construct_type(int id, type * of, type * to)
{
  type * t = (type *)malloc(sizeof(type));
  if (t != NULL)
  {
    t->id = id;
    t->of = of;
    t->to = to;
    t->name = NULL;
  }
  return t;
}

type * construct_pair(type * a, type * b)
{
  if (a == NULL) return b;
  if (b == NULL) return a;
  return construct_type(TYPEID_pair, a, b);
}

type * construct_function(type * a, type * b)
{
  return construct_type(TYPEID_func, a, b);
}

type * unname_type(type * a)
{
  if (a == NULL) return NULL;
  if (a->id >= 128)
    return construct_type(a->id, unname_type(a->of), unname_type(a->to));
  if (a->name == NULL) return a;
  return a->of;
}

int count_type(type * x)  /* returns number of primitives in tuple */
{
  if (x == NULL) return 0;
  if (x->id != TYPEID_pair) return 1;
  return count_type(x->of) + count_type(x->to);
}

type * nth_type(type * x, int n) /* returns one primitive from a tuple */
{
  int l = count_type(x);
  while (n < l && x != NULL && x->id == TYPEID_pair)
  {
    l--;
    x = x->of;
  }
  if (x->id == TYPEID_pair) x = x->to;
  return x;
}

type * domain_type(type * x) /* returns type of arguments to function */
{
  if (x == NULL) return NULL;
  if (x->id == TYPEID_func) return x->of;
  return x;
}

type * range_type(type * x) /* returns type of result of function */
{
  if (x == NULL) return NULL;
  if (x->id == TYPEID_func) return x->to;
  return x;
}

int equivalent_types(type * a, type * b)
{
  if (a == b) return 1;
  if (a == NULL || b == NULL) return 0;
  if (a->name != NULL && b->name != NULL) return (a->name == b->name);
  if (a->id == b->id)
  {
    if (a->id >= 128) return equivalent_types(a->of, b->of) &&
			     equivalent_types(a->to, b->to);
		 else return 1;
  }
  return 0;
}

void print_type(FILE * f, type * t)
{
  if (t==NULL)
  {
    fprintf(f, "void");
    return;
  } else
  if (t->name != NULL)
  {
    fprintf(f, "%s", t->name->id);
  } else
  switch(t->id)
  {
    case TYPEID_type: fprintf(f, "type"); break;
    case TYPEID_int:  fprintf(f, "int"); break;
    case TYPEID_char: fprintf(f, "char"); break;
    case TYPEID_pair: print_type(f, t->of);
		      fprintf(f, ", ");
		      print_type(f, t->to); break;
    case TYPEID_func: fprintf(f, "{");
		      print_type(f, t->of);
		      fprintf(f, " -> ");
		      print_type(f, t->to);
		      fprintf(f, "}"); break;
    case TYPEID_many: fprintf(f, "many ");
		      print_type(f, t->of); break;
    default: fprintf(f, "type error (%d)", t->id);
  }
}

/****************************** SYMBOL TABLE ******************************/

symbol * head = NULL;

/* assumed: preceding call to sym_lookup failed. */
symbol * sym_defn(char * s, type * t)
{
  symbol * n = (symbol *)malloc(sizeof(symbol));
  if (n == NULL)
  {
    fprintf(stderr, "Could not allocate symbol table record\n");
  } else
  {
    n->id = (char *)malloc(strlen(s)+1);
    if (n->id == NULL)
    {
      fprintf(stderr, "Could not allocate lexeme\n");
    } else
    {
      strcpy(n->id, s);
      n->t = t;
      n->next = head;
      head = n;
      return n;
    }
  }
  return NULL;
}

symbol * sym_lookup(char * s)
{
  symbol * h = head;
  while(h != NULL)
  {
    if(!strcmp(s, h->id)) return h;
    h = h->next;
  }
  return NULL;
}

/******************************    SCANNER    ******************************/

FILE * infile;
FILE * outfile;
char   token[256];
int    lino = 1;
int    column = 0;

int isatype(void)
{
  symbol * s = sym_lookup(token);
  if (s != NULL && s->t->id == TYPEID_type) return 1;
  if (tokeq("void") || tokeq("int") || tokeq("char") || tokeq("like")) return 1;
  return 0;
}

void error(char * s)
{
  fprintf(stderr, "Error (line %d, column %d, token '%s'): %s.\n",
	  lino, column, token, s);
}

void scan(void)
{
  char x;
  int i = 0;

  chkeof;
  x = (char)getc(infile); column++;
  chkeof;
  while (x <= ' ')
  {
    if (x == '\n') { lino++; column = 0; }
    x = (char)getc(infile); column++;
    chkeof;
  }

  if (x == '"')
  {
    token[i++] = x;
    x = (char)getc(infile); column++;
    chkeof;
    while (x != '"')
    {
      token[i++] = x;
      x = (char)getc(infile); column++;
      chkeof;
    }
    token[i] = 0;
    return;
  } else
  if (!isspace((int)x) && !feof(infile))
  {
    while (!isspace((int)x) && !feof(infile))
    {
      token[i++] = x;
      x = (char)getc(infile); column++;
      chkeof;
    }
    ungetc(x, infile); column--;
    token[i] = 0;
    return;
  } else
  {
    token[0] = 0;
    return;
  }
}

/******************************    PARSER    ******************************/

type * application(type * func, type * avail)
{
  type * args = NULL;
  int i;

  while(count_type(args) < count_type(domain_type(func))
	&& token[0] != 0)
    args = construct_pair(args, instruction(avail));

  for(i=count_type(domain_type(func));i>=1;i--)
    if(!equivalent_types(debit_type(), nth_type(domain_type(func),i)))
      error("Type mismatch");

  for(i=1;i<=count_type(range_type(func));i++)
    credit_type(nth_type(range_type(func),i));

  return range_type(func);
}

type * instruction(type * avail)
{
  type * mytype = NULL;  /* the type of this instruction */
  symbol * s = sym_lookup(token);
  if (tokeq("do"))
  {
    int argnum;
    scan();
    argnum = atoi(token);
    if (argnum > count_type(avail))
      error("Maximum argument count exceeded");
    scan();
    mytype = application(nth_type(avail, argnum), avail);
    fprintf(outfile, "  (*arg%d)();\n", argnum);
  } else
  if (tokeq("the"))
  {
    scan();
    s = sym_lookup(token);
    scan();
    mytype = s->t;
    fprintf(outfile, "  push((int)&apply_%s);\n", s->id);
    credit_type(mytype);
  } else
  if (tokeq("as"))
  {
    int i = 0;
    scan();
    while(isatype())
      mytype = construct_pair(mytype, parse_type());
    while(i < count_type(unname_type(mytype)))
    {
      i += count_type(unname_type(instruction(avail)));
      debit_type();
    }
    for(i=1;i<=count_type(mytype);i++)
      credit_type(nth_type(mytype,i));
  } else
  if (tokeq("if"))
  {
    type * b = NULL;
    type * c = NULL;
    scan();

    (void)instruction(avail);
    fprintf(outfile, "  if(pop() != 0) {\n");
    (void)debit_type();

    b = instruction(avail);
    fprintf(outfile, "  } else {\n");
    (void)debit_type();

    c = instruction(avail);
    fprintf(outfile, "  }\n");
    (void)debit_type();

    if (!equivalent_types(b, c)) error("Need equivalent types in 'if'");
    mytype = b;
    credit_type(mytype);
  } else
  if (isdigit((int)token[0]))
  {
    int litnum = atoi(token);
    fprintf(outfile, "  push(%d);\n", litnum);
    scan();
    mytype = construct_type(TYPEID_int, NULL, NULL);
    credit_type(mytype);
  } else
  if (token[0] == '\'')
  {
    int litnum = (int)token[1];
    if (litnum==0) litnum = 32;
    fprintf(outfile, "  push(%d);\n", litnum);
    scan();
    mytype = construct_type(TYPEID_char, NULL, NULL);
    credit_type(mytype);
  } else
  if (token[0] == '$')
  {
    int argnum = atoi(token+1);
    if (argnum <= count_type(avail))
    {
      int j;
      type * x = nth_type(avail, argnum);
      if (x->name != NULL)
      {
	for(j = 1; j <= count_type(unname_type(x)); j++)
	  fprintf(outfile, "  push(arg%d_%d);\n", argnum, j);
      } else fprintf(outfile, "  push(arg%d);\n", argnum);
    } else error("Maximum argument count exceeded");
    scan();
    mytype = nth_type(avail, argnum);
    credit_type(mytype);
  } else
  if (s != NULL)
  {
    char name[80];
    strcpy(name, token);
    scan();
    mytype = application(s->t, avail);
    fprintf(outfile, "  apply_%s();\n", name);
  } else
  {
    error("Undefined symbol");
    scan();
  }
  return mytype;
}

type * parse_type(void)
{
  type * t = NULL;
  symbol * s = sym_lookup(token);

  if (s != NULL)
  {
    if (s->t->id == TYPEID_type)
    {
      t = s->t;
      scan();
    }
  } else
  if (tokeq("void")) scan(); else
  if (tokeq("int"))
  {
    t = construct_type(TYPEID_int, NULL, NULL);
    scan();
  } else
  if (tokeq("char"))
  {
    t = construct_type(TYPEID_char, NULL, NULL);
    scan();
  } else
  if (tokeq("like"))
  {
    scan();
    s = sym_lookup(token);
    if (s == NULL) error("Undefined function named in 'like'"); else
    if (s->t->id == TYPEID_type) error("'like' needs function name, not type name");
    t = s->t;
    scan();
  } else
  {
    error("Invalid type");
    scan();
  }
  return t;
}

void definition(void)
{
  type * t = NULL;
  type * u = NULL;
  symbol * s = NULL;

  while (!isatype() && token[0] != 0)
  {
    FILE * f;
    FILE * g = infile;
    int l = lino;
    char fn[256];
    sprintf(fn, "%s.sal", token);
    f = fopen(fn, "r");
    if (f != NULL)
    {
      infile = f;
      scan();
      while(token[0] != 0) definition();
      infile = g;
      fclose(f);
    }
    lino = l;
    scan();
  }

  while(isatype()) t = construct_pair(t, parse_type());

  if(sym_lookup(token) == NULL)
  {
    type * y = NULL;
    char name[80];
    strcpy(name, token);
    scan();
    if(tokeq("type"))
    {
      y = construct_type(TYPEID_type, t, NULL);
      s = sym_defn(name, y);
      y->name = s;
      scan();
    } else
    {
      while(isatype())
        u = construct_pair(u, parse_type());
      y = construct_function(t, u);
      s = sym_defn(name, y);
      fprintf(outfile, "/* ");
      print_type(outfile, y);
      fprintf(outfile, ": */ ");
      if(tokeq("proto"))
      {
        fprintf(outfile, "void apply_%s(void);\n", name);
        scan();
      } else
      if(s != NULL)
      {
	int i = 0, j = 0;
	int is_main = 0;
	int inputs = count_type(t);
	type * v = NULL;

	if(!strcmp(name, "main"))
	{
	  is_main = 1;
	  fprintf(outfile, "int main(int argc, char ** argv)\n{\n");
	  for(i=1; i <= inputs; i++)
	    fprintf(outfile, "  int arg%d = atoi(argv[%d]);\n", i, i);
  	fprintf(outfile, "  if(argc <= %d) { fprintf(stderr, \"%d values needed\"); exit(1); }\n", inputs, inputs);
        } else
	{
	  fprintf(outfile, "void apply_%s(void)\n{\n", name);
	  for(i = inputs; i >= 1; i--)
	  {
	    type * x = nth_type(t, i);
	    if(x->name != NULL)
	    {
	      for(j = count_type(unname_type(x)); j >= 1; j--)
		fprintf(outfile, "  int arg%d_%d = pop();\n", i, j);
	      fprintf(outfile, "    /* ");
	    } else
	    switch(x->id)
	    {
	      case TYPEID_int:  fprintf(outfile, "  int arg%d = pop(); /* ", i); break;
	      case TYPEID_char: fprintf(outfile, "  char arg%d = (char)pop(); /* ", i); break;
	      case TYPEID_func: fprintf(outfile, "  void (*arg%d)(void) = (void (*)())pop(); /* ", i); break;
	      default: fprintf(outfile, "  int arg%d = pop(); /* ", i);
	    }
	    print_type(outfile, x);
	    fprintf(outfile, " */\n");
          }
        }

        while(!isatype() && token[0] != 0)
	  v = construct_pair(v, instruction(t));

        for(i=count_type(range_type(s->t)); i >= 1; i--)
	  if (!equivalent_types(debit_type(), nth_type(range_type(s->t), i)))
	    error("Type mismatch");
        if(tsp != 0) error("Type mismatch");

        if(is_main)
        {
	  fprintf(outfile, "  {\n");
	  for(i=count_type(range_type(s->t)); i >= 1; i--)
	    fprintf(outfile, "    int result%d = pop();\n", i);
	  for(i=1; i <= count_type(range_type(s->t)); i++)
	    fprintf(outfile, "    printf(\"Result #%d: %cd\\n\", result%d);\n", i, '%', i);
	  fprintf(outfile, "  }\n  argv = argv;\n  return 0;\n");
        }

        fprintf(outfile, "}\n\n");
      } else error("Could not allocate symbol");
    }
  } else error("Symbol already declared");
}

/* End of sally.c */
