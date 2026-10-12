/* EX.NO 8 - UDP Server on a lossy network (drops 25% data, 10% ACKs)
   RUN THIS FIRST. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <winsock2.h>
#define socklen_t int
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define SOCKET int
#endif

#define PORT 6000
#define DATA_LOSS 25    /* % of data packets dropped */
#define ACK_LOSS 10     /* % of ACKs dropped */

enum { DATA = 1, ACK, START, START_ACK, FIN, DONE };
typedef struct { int type, seq; char data[256]; } Pkt;

int main()
{
    SOCKET s;
    struct sockaddr_in addr, cli;
    socklen_t clen;
    Pkt p, r;
    char msg[256] = "";
    int expected = 0, printed = 0;
#ifdef _WIN32
    WSADATA w;
    WSAStartup(MAKEWORD(2, 2), &w);
#endif
    setvbuf(stdout, NULL, _IONBF, 0);   /* show output immediately */
    srand(7);   /* same losses every run */
    s = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (bind(s, (struct sockaddr *)&addr, sizeof addr) < 0) {
        printf("Bind failed - port %d busy, close the old server window\n", PORT);
        return 1;
    }
    printf("UDP server listening on 127.0.0.1:%d\n", PORT);

    while (1) {
        clen = sizeof cli;
        if (recvfrom(s, (char *)&p, sizeof p, 0, (struct sockaddr *)&cli, &clen) <= 0)
            continue;   /* Windows reports errors from old clients here - ignore */
        memset(&r, 0, sizeof r);

        if (p.type == START) {
            msg[0] = '\0';
            expected = 0;
            printed = 0;
            printf("\nNew client session\n");
            r.type = START_ACK;
        } else if (p.type == FIN) {
            if (!printed) { printf("\nFull message received: %s\n", msg); printed = 1; }
            r.type = DONE;
            strcpy(r.data, msg);
        } else {
            if (rand() % 100 < DATA_LOSS) {
                printf("  seq=%d DROPPED (simulated loss)\n", p.seq);
                continue;
            }
            if (p.seq == expected) {
                strcat(msg, p.data);
                expected++;
                printf("  seq=%d received \"%s\"\n", p.seq, p.data);
            } else {
                printf("  seq=%d duplicate (ACK was lost), re-ACK\n", p.seq);
            }
            if (rand() % 100 < ACK_LOSS) {
                printf("  ACK %d DROPPED (simulated loss)\n", p.seq);
                continue;
            }
            r.type = ACK;
            r.seq = p.seq;
        }
        sendto(s, (char *)&r, sizeof r, 0, (struct sockaddr *)&cli, clen);
    }
    return 0;
}
