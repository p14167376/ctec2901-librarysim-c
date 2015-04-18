ifdef NDEBUG
  ndebug=-DNDEBUG
else
  ndebug=
endif

sources = avl_any.h avl_any.c smalloc.h trace.h library.h library.c makefile
derived = avl_any.o library.o library

all: library

library: avl_any.o library.o
	gcc -o library avl_any.o library.o -L../../lib -llinked_clists -lm

library.o: library.h library.c $(sources)
	gcc -c library.c -I../../include

avl_any.o: avl_any.h avl_any.c
	gcc -c avl_any.c -I../../include


clean:
	/bin/rm -f $(derived)

install:
	make clean
	make all

