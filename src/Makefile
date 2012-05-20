# GNU Makefile for sally
# Created 2003.1104 Chris Pressey, Cat's Eye Technologies

# CC=gcc
CFLAGS=-ansi -pedantic -g -Wall -O

OBJECTS= sally.o sally2c.o

all: ../lib/libsally.a ../bin/sally2c

sally.o: sally.c sally.h

sally2c.o: sally2c.c sally.h

runtime.o: runtime.c

../lib/libsally.a: runtime.o
	ar rc ../lib/libsally.a runtime.o
	ranlib ../lib/libsally.a

../bin/sally2c: $(OBJECTS)
	$(CC) -o../bin/sally2c $(OBJECTS)
	strip ../bin/sally2c

clean:
	rm *.o
	rm ../bin/*
	rm ../lib/*
