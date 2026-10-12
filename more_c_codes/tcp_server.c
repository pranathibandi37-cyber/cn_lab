/* EX.NO 8 - TCP Server with Traffic Control (flow control + congestion signal)
   RUN THIS FIRST. */
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define SOCKET int
#define closesocket close
#endif

#define PORT 5000
#define RWND 3      /* receiver window advertised to client (flow control) */
#define DROP 5      /* segment 5 is "lost" once (simulated congestion) */

enum { DATA = 1, ACK, NAK, FIN, DONE };
typedef struct { int type, seq, rwnd; char data[256]; } Pkt;

int recv_all(SOCKET s, char *buf, int len)
{
    int got = 0, r;
    while (got < len) {
        r = recv(s, buf + got, len - got, 0);
        if (r <= 0) return 0;
        got += r;
    }
    return 1;
}

int main()
{
    SOCKET srv, cli;
    struct sockaddr_in addr;
    Pkt p, r;
    char msg[256];
    int expected, dropped, opt = 1;
#ifdef _WIN32
    WSADATA w;
    WSAStartup(MAKEWORD(2, 2), &w);
#endif
    setvbuf(stdout, NULL, _IONBF, 0);   /* show output immediately */
    srv = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof opt);
    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (bind(srv, (struct sockaddr *)&addr, sizeof addr) < 0) {
        printf("Bind failed - port %d busy, close the old server window\n", PORT);
        return 1;
    }
    listen(srv, 5);
    printf("TCP server listening on 127.0.0.1:%d\n", PORT);

    while (1) {
        cli = accept(srv, NULL, NULL);
        printf("\nClient connected\n");
        msg[0] = '\0';
        expected = 0;
        dropped = 0;
        while (recv_all(cli, (char *)&p, sizeof p)) {
            memset(&r, 0, sizeof r);
            if (p.type == FIN) {
                printf("\nFull message received: %s\n", msg);
                r.type = DONE;
                strcpy(r.data, msg);
                send(cli, (char *)&r, sizeof r, 0);
                break;
            }
            r.seq = p.seq;
            r.rwnd = RWND;
            if (p.seq == DROP && !dropped) {
                dropped = 1;
                r.type = NAK;
                printf("  seq=%d LOST (simulated congestion) -> NAK\n", p.seq);
            } else if (p.seq == expected) {
                strcat(msg, p.data);
                expected++;
                r.type = ACK;
                printf("  seq=%d accepted \"%s\" -> ACK (rwnd=%d)\n", p.seq, p.data, RWND);
            } else {
                r.type = NAK;
                printf("  seq=%d out of order (expected %d), discarded -> NAK\n", p.seq, expected);
            }
            send(cli, (char *)&r, sizeof r, 0);
        }
        closesocket(cli);
        printf("Client disconnected. Waiting for next client...\n");
    }
    return 0;
}
