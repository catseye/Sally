/*
  sally.h: Sally language compiler header
  (c)2000 Cat's-Eye Technologies.  All rights reserved.
*/

#include <stdio.h>
#include <stdlib.h>

/******************************    RUNTIME    ******************************/

extern int stack[];
extern int sp;

extern int pop(void);
extern void push(int i);

/******************************  TYPE SYSTEM  ******************************/

#define TYPEID_type    0
#define TYPEID_int     1
#define TYPEID_char    2
#define TYPEID_pair  128
#define TYPEID_func  129
#define TYPEID_many  130

typedef struct Type
{
  int id;
  struct Type * of;     /* for mappings (id >= 128) */
  struct Type * to;     /* for mappings (id >= 128) */
  struct Symbol * name; /* for named types & name equivalence */
} type;

extern type * typestack[];
extern int tsp;

extern type * construct_type(int id, type * of, type * to);
extern type * construct_pair(type * a, type * b);
extern type * construct_function(type * a, type * b);
extern int count_type(type * x);  /* returns number of primitives in tuple */
extern type * nth_type(type * x, int n); /* returns one primitives from a tuple */
extern type * domain_type(type * x);
extern type * range_type(type * x);
extern int equivalent_types(type * a, type * b);
extern void print_type(FILE * f, type * t);

/****************************** SYMBOL TABLE ******************************/

typedef struct Symbol
{
  char * id;
  type * t;
  struct Symbol * next;
} symbol;

extern symbol * head;

extern symbol * sym_defn(char * s, type * t);
extern symbol * sym_lookup(char * s);

/******************************    SCANNER    ******************************/

extern FILE * infile;
extern FILE * outfile;
extern char   token[];
extern int    lino;

#define tokeq(x)    (!strcmp(token,x))
#define tokne(x)    (strcmp(token,x))
#define chkeof      if (feof(infile)) { token[0] = 0; return; }

extern int isatype(void);
extern void error(char * s);
extern void scan(void);

/******************************    PARSER    ******************************/

extern type * instruction(type * avail);
extern type * parse_type(void);
extern void definition(void);

/* End of sally.h */
