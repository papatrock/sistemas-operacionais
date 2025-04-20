CFLAGS = -Wall -g

Objs = pingpong-scheduler.o ppos_core.o queue.o

all: pingpong-scheduler

debug: CFLAGS += -D_DEBUG_ -g

pingpong-scheduler: $(Objs)
	gcc $(Objs) $(CFLAGS) -o pingpong-scheduler

pingpong-scheduler.o: pingpong-scheduler.c ppos_core.c ppos.h
	gcc $(CFLAGS) -c pingpong-scheduler.c

ppos_core.o: ppos_core.c ppos.h
	gcc $(CFLAGS) -c ppos_core.c

queue.o: queue.c queue.h
	gcc $(CFLAGS) -c queue.c

clean:
	rm -rf *~ *.o

purge: clean
	rm -rf pingpong-scheduler

tar:
	tar -czf pingpong-scheduler.tgz *.c *.h Makefile
