CFLAGS = -Wall -g

Objs = pingpong-preempcao.o ppos_core.o queue.o

all: pingpong-preempcao

debug: CFLAGS += -D_DEBUG_ -g

pingpong-preempcao: $(Objs)
	gcc $(Objs) $(CFLAGS) -o pingpong-preempcao

pingpong-preempcao.o: pingpong-preempcao.c ppos_core.c ppos.h
	gcc $(CFLAGS) -c pingpong-preempcao.c

ppos_core.o: ppos_core.c ppos.h
	gcc $(CFLAGS) -c ppos_core.c

queue.o: queue.c queue.h
	gcc $(CFLAGS) -c queue.c

clean:
	rm -rf *~ *.o

purge: clean
	rm -rf pingpong-preempcao

tar:
	tar -czf pingpong-preempcao.tgz *.c *.h Makefile
