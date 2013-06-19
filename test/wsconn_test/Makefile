.PHONY : all clean 

CFLAGS = -ggdb -Wall -fPIC
LIBS = ../../gbase.a
INCLUDES = ../../

all: server

server: server.o
	gcc -o server $(CFLAGS) $^ $(LIBS)

server.o: server.c
	gcc -c $(CFLAGS) $< -o $@ -I$(INCLUDES)
	
clean:
	rm -rf *.o server
