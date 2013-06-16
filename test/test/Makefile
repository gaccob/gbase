.PHONY : all clean 

CFLAGS = -ggdb -Wall -fPIC
LDFLAGS = -lpthread
LIBS = ../../gbase.a
INCLUDES = ../../

all: test

test: test.o
	gcc -o test $(CFLAGS) $^ $(LDFLAGS) $(LIBS)

test.o: test.c
	gcc -c $(CFLAGS) $< -o $@ -I$(INCLUDES)

clean:
	rm -rf *.o test
