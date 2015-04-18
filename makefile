ifdef NDEBUG
  ndebug=-DNDEBUG
else
  ndebug=
endif

sources = makefile\
	smalloc.h trace.h\
	mvar.h mvar.c\
	avl_any.h avl_any.c\
	shutdown.h shutdown.c\
	msg_queue.h msg_queue.c\
	library.h library.c
derived = mvar.o avl_any.o msg_queue.o library.o library

all: library

library: mvar.o avl_any.o library.o
	gcc -o library avl_any.o library.o -L../../lib -llinked_clists -llinked_queues -lm

library.o: library.h library.c $(sources)
	gcc -c library.c -I../../include

%.o : %.c
	gcc -c $< -I../../include

clean:
	/bin/rm -f $(derived)

install:
	make clean
	make all

