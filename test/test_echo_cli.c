#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include "core/os_def.h"
#include "net/sock.h"
#include "test.h"

int
test_echo_cli() {
    sock_t sock = sock_tcp();
    assert(sock >= 0);

    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    int res = sock_nonblock_connect(sock, ECHO_IP, ECHO_PORT, tv);
    assert(0 == res);

    printf("\n===========================================\n");
    while (1) {
        printf("send msg to server(q->quit, enter->input): ");

        char buffer[1024];
        fgets(buffer, sizeof(buffer) - 1, stdin);
        size_t buflen = strlen(buffer);
        if (buflen == 2 && buffer[0] == 'q' && buffer[1] == '\n') {
            break;
        }
        size_t nsend = 0;
        while (nsend < buflen) {
            res = write(sock, buffer, buflen);
            if(res > 0) {
                nsend += res;
            } else {
                usleep(100);
            }
        }

        memset(buffer, 0, sizeof(buffer));
        size_t dest_buflen = buflen;
        buflen = sizeof(buffer);
        size_t nread = 0;
        while (nread < dest_buflen) {
            res = read(sock, buffer + nread, sizeof(buffer) - nread);
            if (res > 0) {
                nread += res;
            } else if (res == 0) {
                printf("\necho server quit.\n");
                return 0;
            } else {
                usleep(1);
            }
        }

        printf("\necho: %s=========================\n", buffer);
    }

    sock_close(sock);
    return 0;
}

