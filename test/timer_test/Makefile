.PHONY : all clean 

CFLAGS = -ggdb -Wall -fPIC
LIBS = ../../gbase.a
INCLUDES = ../../

all: test

test: test.o
	gcc -o test $(CFLAGS) $^ $(LIBS)

test.o: test.c
	gcc -c $(CFLAGS) $< -o $@ -I$(INCLUDES)

clean:
	rm -rf *.o test
