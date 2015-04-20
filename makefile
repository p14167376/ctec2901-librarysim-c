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
	borrower.h borrower.c\
	librarian.h librarian.c\
	library.h library.c\
	sim.h sim.c
objfiles = mvar.o set_ints.o avl_any.o shutdown.o msg_queue.o borrower.o librarian.o library.o sim.o
derived = $(objfiles) sim

all: sim

sim: $(objfiles)
	gcc -o sim $(objfiles) -L$(HOME)/lib  -llist_sets -llinked_clists -llinked_queues -lm -pthread

%.o : %.c
	gcc -c $< -I$(HOME)/include

clean:
	/bin/rm -f $(derived)

install:
	make clean
	make all

