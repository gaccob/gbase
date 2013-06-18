.PHONY : all test clean

CFLAGS = -pg -ggdb -Wall
LDFLAGS = -lpthread -ldl -lrt -Wl -E
SRCS = os/atom.c \
	os/log.c \
	os/spin.c \
	os/thread.c \
	os/util.c \
	net/sock.c \
	net/reactor.c \
	net/reactor_kqueue.c \
	net/reactor_epoll.c \
	net/reactor_select.c \
	net/acceptor.c \
	net/connector.c \
	net/serialize.c \
	net/wsconn.c \
	ds/array.c \
	ds/buddy.c \
	ds/connbuffer.c \
	ds/hash.c \
	ds/idtable.c \
	ds/heap.c \
	ds/timer.c \
	ds/bitset.c \
	ds/rbtree.c \
	ds/rqueue.c \
	ds/rbuffer.c \
	ds/sha1.c \
	ds/md5.c \
	ds/slist.c
OBJS = $(patsubst %.c, %.o, $(SRCS))
TARGET = gbase.a

TEST = test/atom_test \
	test/bitset_test \
	test/heap_test \
	test/rbtree_test \
	test/spin_test \
	test/thread_test \
	test/timer_test \
	test/slist_test \
	test/tcp_test \
	test/test

.PHONY: all clean

all : $(TARGET) test

$(TARGET) : $(OBJS)
	ar -r -s $@ $^

%.o: %.c
	gcc -c $(CFLAGS) $< -o $@ -I.

test:
	for i in $(TEST); do $(MAKE) -C $$i || exit 1; done

clean :
	rm *.a
	rm */*.o
	for i in $(TEST); do $(MAKE) -C $$i clean || exit 1; done
