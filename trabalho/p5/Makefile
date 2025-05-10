CFLAGS = -Wall -g

Objs = ppos_core.o queue.o

all: pingpong-preempcao

debug: clean
	make CFLAGS="$(CFLAGS) -DDEBUG" all

pingpong-preempcao: pingpong-preempcao.o $(Objs)
	gcc pingpong-preempcao.o $(Objs) $(CFLAGS) -o pingpong-preempcao

pingpong-preempcao.o: pingpong-preempcao.c ppos_core.c ppos.h
	gcc $(CFLAGS) -c pingpong-preempcao.c

stress: pingpong-preempcao-stress

pingpong-preempcao-stress: pingpong-preempcao-stress.o $(Objs)
	gcc pingpong-preempcao-stress.o $(Objs) $(CFLAGS) -o pingpong-preempcao-stress

pingpong-preempcao-stress.o: pingpong-preempcao-stress.c ppos_core.c ppos.h
	gcc $(CFLAGS) -c pingpong-preempcao-stress.c

ppos_core.o: ppos_core.c ppos.h
	gcc $(CFLAGS) -c ppos_core.c

queue.o: queue.c queue.h
	gcc $(CFLAGS) -c queue.c

clean:
	rm -rf *~ *.o

purge: clean
	rm -rf pingpong-preempcao pingpong-preempcao-stress

tar:
	tar -czf pingpong-preempcao.tgz *.c *.h Makefile