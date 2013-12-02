#include "tcp_test.h"
#include <string.h>
#include <stdio.h>

int main()
{
    int res;
    sock_t sock;
    char buffer[1024];
    size_t buflen, dest_buflen;
    size_t nsend, nread;

    sock = sock_tcp();
    if(sock < 0)
        return -1;

    res = sock_connect(sock, server_addr, server_port);
    if(res < 0)
        return -1;

    printf("\n===========================================\n");
    while(1)
    {
        printf("send msg to server(q->quit, enter->input): ");

        /* get input */
        fgets(buffer, sizeof(buffer) - 1, stdin);
        buflen = strlen(buffer);
        if(buflen == 2 && buffer[0] == 'q' && buffer[1] == '\n')
            break;
        nsend = 0;
        while(nsend < buflen)
        {
            res = sock_write(sock, buffer, buflen);
            if(res > 0)
                nsend += res;
            else
                SLEEP(1);
        }

        memset(buffer, 0, sizeof(buffer));
        dest_buflen = buflen;
        buflen = sizeof(buffer);
        nread = 0;
        while(nread < dest_buflen)
        {
            res = sock_read(sock, buffer + nread, sizeof(buffer) - nread);
            if(res > 0)
                nread += res;
            else
                SLEEP(1);
        }

        printf("\necho: %s===========================================\n", buffer);
    }

    sock_close(sock);
    return 0;
}

