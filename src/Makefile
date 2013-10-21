# GNU Makefile for sally
# Created 2003.1104 Chris Pressey, Cat's Eye Technologies

LIBDIR?=../lib
BINDIR?=../bin
LIB?=$(LIBDIR)/libsally.a
PROG?=$(BINDIR)/sally2c
CC?=gcc
CFLAGS?=-ansi -pedantic -g -Wall -O

OBJECTS= sally.o sally2c.o

all: $(LIB) $(PROG)

sally.o: sally.c sally.h

sally2c.o: sally2c.c sally.h

runtime.o: runtime.c

$(LIBDIR):
	mkdir -p $(LIBDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

$(LIB): $(LIBDIR) runtime.o
	ar rc $(LIB) runtime.o
	ranlib $(LIB)

$(PROG): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(PROG)
	strip $(PROG)

clean:
	rm -f *.o $(PROG) $(LIB)
