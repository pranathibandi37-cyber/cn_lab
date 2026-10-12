/* EX.NO 8 - UDP Client with Traffic Control
   Leaky bucket      -> traffic shaping (constant output rate)
   Stop-and-Wait ARQ -> reliability over UDP (seq no + ACK + timeout + retransmit)
   RUN AFTER THE SERVER. */
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <winsock2.h>
#define sleep_ms(x) Sleep(x)
long now_ms() { return (long)GetTickCount(); }
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#define SOCKET int
#define closesocket close
#define sleep_ms(x) usleep((x) * 1000)
long now_ms() { struct timeval t; gettimeofday(&t, NULL); return t.tv_sec * 1000L + t.tv_usec / 1000; }
#endif

#define PORT 6000
#define CHUNK 4         /* characters per packet */
#define INTERVAL 200    /* leaky bucket: 1 packet every 200 ms */
#define MAX_RETRIES 30

enum { DATA = 1, ACK, START, START_ACK, FIN, DONE };
typedef struct { int type, seq; char data[256]; } Pkt;

SOCKET s;
struct sockaddr_in srv;
long next_time;
int sent = 0, retx = 0;

void leaky_wait()
{
    long now = now_ms();
    if (now < next_time) sleep_ms(next_time - now);
    next_time = (now > next_time ? now : next_time) + INTERVAL;
}

/* send p, keep retransmitting until a reply of type `want` (same seq for ACK) arrives */
int send_reliable(Pkt *p, int want, Pkt *r)
{
    int attempt;
    for (attempt = 1; attempt <= MAX_RETRIES; attempt++) {
        leaky_wait();
        sendto(s, (char *)p, sizeof *p, 0, (struct sockaddr *)&srv, sizeof srv);
        sent++;
        if (attempt > 1) retx++;
        while (recvfrom(s, (char *)r, sizeof *r, 0, NULL, NULL) > 0)
            if (r->type == want && (want != ACK || r->seq == p->seq))
                return 1;   /* otherwise: stale ACK from an old retransmission, ignore */
        printf("    timeout (attempt %d) -> retransmit\n", attempt);
    }
    printf("No reply from server. Is the server running?\n");
    return 0;
}

int main()
{
    Pkt p, r;
    char msg[200];
    int n, seq;
#ifdef _WIN32
    DWORD tv = 500;   /* 500 ms ACK timeout */
    WSADATA w;
    WSAStartup(MAKEWORD(2, 2), &w);
#else
    struct timeval tv = {0, 500000};
#endif
    setvbuf(stdout, NULL, _IONBF, 0);   /* show output immediately */
    printf("Enter message: ");
    fgets(msg, sizeof msg, stdin);
    msg[strcspn(msg, "\r\n")] = '\0';
    if (msg[0] == '\0') strcpy(msg, "Hello from UDP client 24BCE2079");
    n = (strlen(msg) + CHUNK - 1) / CHUNK;

    s = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof tv);
    memset(&srv, 0, sizeof srv);
    srv.sin_family = AF_INET;
    srv.sin_port = htons(PORT);
    srv.sin_addr.s_addr = inet_addr("127.0.0.1");
    next_time = now_ms();
    printf("Sending %d packets, leaky bucket = 1 packet / %d ms\n", n, INTERVAL);

    memset(&p, 0, sizeof p);
    p.type = START;
    if (!send_reliable(&p, START_ACK, &r)) { getchar(); return 1; }

    for (seq = 0; seq < n; seq++) {
        memset(&p, 0, sizeof p);
        p.type = DATA;
        p.seq = seq;
        strncpy(p.data, msg + seq * CHUNK, CHUNK);
        printf("  send seq=%d data=\"%s\"\n", seq, p.data);
        if (!send_reliable(&p, ACK, &r)) { getchar(); return 1; }
        printf("  ACK %d\n", seq);
    }

    memset(&p, 0, sizeof p);
    p.type = FIN;
    if (!send_reliable(&p, DONE, &r)) { getchar(); return 1; }
    closesocket(s);

    printf("\nSent     : %s\n", msg);
    printf("Received : %s\n", r.data);
    printf("Packets  : %d data, %d transmissions, %d retransmissions\n", n, sent, retx);
    printf("RESULT   : %s\n", strcmp(msg, r.data) == 0 ? "SUCCESS - 100% delivered" : "MISMATCH");
    printf("\nPress Enter to exit");
    getchar();
    return 0;
}
