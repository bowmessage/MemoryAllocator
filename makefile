# makefile

all: memtest

ackerman.o: ackerman.c ackerman.h 
	g++ -c -g ackerman.c

my_allocator.o : my_allocator.c my_allocator.h
	g++ -c -g my_allocator.c

memtest: memtest.c ackerman.o my_allocator.o
	g++ -o memtest memtest.c ackerman.o my_allocator.o

