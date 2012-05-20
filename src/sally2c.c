/*
  sally2c.c: Sally to C compiler (driver unit)
  (c)2000 Cat's-Eye Technologies.  All rights reserved.
*/

#include <string.h>
#include "sally.h"

/****************************** MAIN PROGRAM ******************************/

int main(int argc, char ** argv)
{
  infile = stdin;  outfile = stdout;
  fprintf(outfile, "#include \"sally.h\"\n\n");
  scan();
  while(token[0] != 0) definition();
  argc = argc; argv = argv;
  return 0;
}

/* End of sally2c.c */
