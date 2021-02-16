CC = gcc
CFLAGS = -Wall -std=c99

msh: msh.o; $(CC) $(CFLAGS) msh.o -o msh

msh.o: msh.c; $(CC) $(CFLAGS) -c msh.c

clean:; rm -rf *.o ./msh

