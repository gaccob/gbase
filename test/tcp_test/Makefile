.PHONY : all clean 

CFLAGS = -ggdb -Wall -fPIC
LIBS = ../../gbase.a
INCLUDES = ../../

all: client server

client: client.o
	gcc -o client $(CFLAGS) $^ $(LIBS)

client.o: client.c
	gcc -c $(CFLAGS) $< -o $@ -I$(INCLUDES)

server: server.o
	gcc -o server $(CFLAGS) $^ $(LIBS)

server.o: server.c
	gcc -c $(CFLAGS) $< -o $@ -I$(INCLUDES)
	
clean:
	rm -rf *.o server client
