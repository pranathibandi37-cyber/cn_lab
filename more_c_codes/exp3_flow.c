/* EX.NO 3 - Flow Control Mechanisms
   1. Stop-and-Wait  2. Sliding Window  3. Go-Back-N (corrupted frame)
   4. Go-Back-N (lost frame / lost ACK)  5. Selective Repeat */
#include <stdio.h>

int min(int a, int b) { return a < b ? a : b; }

void stop_and_wait(int n, int bad)
{
    int i, done = 0;
    for (i = 1; i <= n; i++) {
        printf("Frame %d sent\n", i);
        if (i == bad && !done) {
            done = 1;
            printf("  Frame %d LOST -> no ACK -> Timeout\n", i);
            printf("Frame %d re-sent\n", i);
        }
        printf("  Frame %d received, ACK %d sent\n", i, i);
        printf("ACK %d received\n\n", i);
    }
}

void sliding_window(int n, int w)
{
    int base = 1, end, i;
    while (base <= n) {
        end = min(base + w - 1, n);
        printf("Window [%d - %d]: ", base, end);
        for (i = base; i <= end; i++) printf("Frame %d sent  ", i);
        printf("\n  Receiver got frames %d to %d, ACK %d sent\n", base, end, end);
        printf("  ACK %d received -> window slides\n\n", end);
        base = end + 1;
    }
}

/* type: 1 = corrupted frame, 2 = lost frame, 3 = lost ACK */
void go_back_n(int n, int w, int bad, int type)
{
    int base = 1, end, i, done = 0, failed;
    while (base <= n) {
        end = min(base + w - 1, n);
        printf("Window [%d - %d]\n", base, end);
        failed = 0;
        for (i = base; i <= end; i++) {
            printf("  Frame %d sent", i);
            if (failed) {
                printf(" -> receiver DISCARDS (out of order)\n");
            } else if (i == bad && !done && type != 3) {
                done = 1;
                failed = 1;
                printf(type == 1 ? " -> CORRUPTED, receiver discards, NAK %d sent\n"
                                 : " -> LOST\n", i);
            } else if (i == bad && !done && type == 3) {
                done = 1;
                printf(" -> received, but ACK %d LOST\n", i);
                if (i == end) {
                    printf("  Timeout -> Frame %d re-sent -> duplicate discarded, ACK %d sent\n", i, i);
                } else {
                    printf("  (next ACK is cumulative, so frame %d is still confirmed)\n", i);
                }
            } else {
                printf(" -> received, ACK %d\n", i);
            }
        }
        if (failed) {
            printf("  %s -> GO BACK to frame %d and resend %d to %d\n\n",
                   type == 1 ? "NAK received" : "Timeout", bad, bad, min(bad + w - 1, n));
            base = bad;
        } else {
            printf("  ACK %d received -> window slides\n\n", end);
            base = end + 1;
        }
    }
}

void selective_repeat(int n, int w, int bad)
{
    int base = 1, end, i, done = 0, lost;
    while (base <= n) {
        end = min(base + w - 1, n);
        printf("Window [%d - %d]\n", base, end);
        lost = 0;
        for (i = base; i <= end; i++) {
            if (i == bad && !done) {
                done = 1;
                lost = 1;
                printf("  Frame %d sent -> LOST\n", i);
            } else {
                printf("  Frame %d sent -> received%s, ACK %d\n", i, lost ? " and BUFFERED" : "", i);
            }
        }
        if (lost) {
            printf("  Timeout for frame %d -> resend ONLY frame %d\n", bad, bad);
            printf("  Frame %d received, ACK %d -> frames %d to %d delivered in order\n", bad, bad, bad, end);
        }
        printf("  Window slides\n\n");
        base = end + 1;
    }
}

int main()
{
    int ch, n, w = 1, bad = 0, t = 2;
    printf("1.Stop-and-Wait  2.Sliding Window  3.Go-Back-N (corrupted frame)\n");
    printf("4.Go-Back-N (lost frame / lost ACK)  5.Selective Repeat\nEnter choice: ");
    scanf("%d", &ch);
    printf("Enter number of frames: ");
    scanf("%d", &n);
    if (ch != 1) {
        printf("Enter window size: ");
        scanf("%d", &w);
    }
    if (ch == 4) {
        printf("1.Lost frame  2.Lost ACK : ");
        scanf("%d", &t);
        t = (t == 1) ? 2 : 3;
    }
    if (ch != 2) {
        printf("Enter frame number to %s: ", ch == 3 ? "corrupt" : "lose");
        scanf("%d", &bad);
    }
    printf("\n");
    switch (ch) {
    case 1: stop_and_wait(n, bad); break;
    case 2: sliding_window(n, w); break;
    case 3: go_back_n(n, w, bad, 1); break;
    case 4: go_back_n(n, w, bad, t); break;
    case 5: selective_repeat(n, w, bad); break;
    default: printf("Invalid choice\n"); return 0;
    }
    printf("RESULT: All %d frames delivered successfully\n", n);
    return 0;
}
