CC=gcc
CFLAGS=`llvm-config --cflags`
LD=g++
LDFLAGS=`llvm-config --libs --cflags --ldflags core analysis executionengine mcjit interpreter native`

main: main.o
	$(LD) main.o $(LDFLAGS) -g3 -o main

main.o: main.c
	$(CC) $(CFLAGS) -g3 -c main.c

clean:
	-rm -rf main.o main
.PHONY: clean
