#Compile with some extra warnings, no -pedantic because we don't hate ourselves
CFLAGS=-c -Wall -Wextra

#This should work for most Linux distros, I think
LIBFLAGS=-lpthread -lm

#Actual target rules
all: ghs

ghs: main.o neighlist.o msgqueue.o node.o algorithm.o
	gcc main.o node.o algorithm.o neighlist.o msgqueue.o -o ghs $(LIBFLAGS)

main.o: main.c
	gcc $(CFLAGS) main.c

neighlist.o: neighlist.c
	gcc $(CFLAGS) neighlist.c

node.o: node.c
	gcc $(CFLAGS) node.c

algorithm.o: algorithm.c
	gcc $(CFLAGS) algorithm.c

msgqueue.o: msgqueue.c
	gcc $(CFLAGS) msgqueue.c

clean:
	rm *.o *.log ghs*
