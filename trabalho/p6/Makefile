CFLAGS = -Wall -g

Objs = ppos_core.o queue.o

all: ppos

debug: clean
	make CFLAGS="$(CFLAGS) -DDEBUG" all

ppos: main.o $(Objs)
	gcc main.o $(Objs) $(CFLAGS) -o ppos

main.o: main.c ppos_core.c ppos.h
	gcc $(CFLAGS) -c main.c

ppos_core.o: ppos_core.c ppos.h
	gcc $(CFLAGS) -c ppos_core.c

queue.o: queue.c queue.h
	gcc $(CFLAGS) -c queue.c

clean:
	rm -rf *~ *.o

purge: clean
	rm -rf ppos

tar:
	mkdir -p p6
	cp *.c *.h Makefile p6/
	tar -czf p6.tgz p6
	rm -rf p6