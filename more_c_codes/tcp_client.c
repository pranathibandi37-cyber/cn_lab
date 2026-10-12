/* EX.NO 8 - TCP Client with Traffic Control
   Token bucket      -> traffic shaping (rate limit, small bursts allowed)
   Sliding window    -> flow control (window = min(cwnd, rwnd))
   Slow start / AIMD -> congestion control (cwnd doubles, then +1, reset on loss)
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

#define PORT 5000
#define CHUNK 4     /* characters per segment */

enum { DATA = 1, ACK, NAK, FIN, DONE };
typedef struct { int type, seq, rwnd; char data[256]; } Pkt;

/* token bucket: 5 packets/sec, burst of 3 */
double tokens = 3, rate = 5, cap = 3;
long last;

void bucket_consume()
{
    long now;
    while (1) {
        now = now_ms();
        tokens += (now - last) * rate / 1000.0;
        if (tokens > cap) tokens = cap;
        last = now;
        if (tokens >= 1) { tokens -= 1; return; }
        printf("    [token bucket] empty, waiting...\n");
        sleep_ms(200);
    }
}

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
    SOCKET s;
    struct sockaddr_in addr;
    Pkt p, r;
    char msg[200];
    int n, seq, win, end, acked, lost;
    int cwnd = 1, ssthresh = 8, rwnd = 4, base = 0, rnd = 0;
#ifdef _WIN32
    WSADATA w;
    WSAStartup(MAKEWORD(2, 2), &w);
#endif
    setvbuf(stdout, NULL, _IONBF, 0);   /* show output immediately */
    printf("Enter message: ");
    fgets(msg, sizeof msg, stdin);
    msg[strcspn(msg, "\r\n")] = '\0';
    if (msg[0] == '\0') strcpy(msg, "Hello from TCP client 24BCE2079");
    n = (strlen(msg) + CHUNK - 1) / CHUNK;

    s = socket(AF_INET, SOCK_STREAM, 0);
    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (connect(s, (struct sockaddr *)&addr, sizeof addr) < 0) {
        printf("Server not running. Start the server first.\n");
        getchar();
        return 1;
    }
    printf("Connected | message split into %d segments\n", n);
    last = now_ms();

    while (base < n) {
        win = cwnd < rwnd ? cwnd : rwnd;
        end = base + win > n ? n : base + win;
        printf("\nRound %d: cwnd=%d rwnd=%d ssthresh=%d -> window=%d%s\n", ++rnd, cwnd, rwnd,
               ssthresh, win, rwnd < cwnd ? "  <- limited by receiver (flow control)" : "");

        for (seq = base; seq < end; seq++) {
            bucket_consume();
            memset(&p, 0, sizeof p);
            p.type = DATA;
            p.seq = seq;
            strncpy(p.data, msg + seq * CHUNK, CHUNK);
            send(s, (char *)&p, sizeof p, 0);
            printf("  sent seq=%d data=\"%s\"\n", seq, p.data);
        }

        acked = base;
        lost = -1;
        for (seq = base; seq < end; seq++) {
            recv_all(s, (char *)&r, sizeof r);
            rwnd = r.rwnd;
            if (r.type == ACK && lost < 0) {
                acked = r.seq + 1;
                printf("  ACK %d\n", r.seq);
            } else if (lost < 0) {
                lost = r.seq;
                printf("  NAK %d (packet lost)\n", r.seq);
            }
        }

        if (lost >= 0) {
            ssthresh = cwnd / 2 > 1 ? cwnd / 2 : 1;
            cwnd = 1;
            printf("  CONGESTION -> ssthresh=%d, cwnd=1, resend from seq %d (Go-Back-N)\n", ssthresh, acked);
            base = acked;
        } else {
            base = end;
            if (cwnd < ssthresh) { cwnd *= 2; printf("  slow start -> cwnd=%d\n", cwnd); }
            else { cwnd++; printf("  congestion avoidance -> cwnd=%d\n", cwnd); }
        }
    }

    memset(&p, 0, sizeof p);
    p.type = FIN;
    send(s, (char *)&p, sizeof p, 0);
    recv_all(s, (char *)&r, sizeof r);
    closesocket(s);

    printf("\nSent     : %s\n", msg);
    printf("Received : %s\n", r.data);
    printf("RESULT   : %s\n", strcmp(msg, r.data) == 0 ? "SUCCESS - 100% delivered" : "MISMATCH");
    printf("\nPress Enter to exit");
    getchar();
    return 0;
}
