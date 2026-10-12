// Q6: Flow control using Go-Back-N (corrupted frame, lost frame, lost ACK)
#include <stdio.h>
int main(){
    int n, w, ch, x, i, base = 1, end, done = 0;
    printf("1. Corrupted frame\n2. Lost frame\n3. Lost ACK\nEnter choice: ");
    scanf("%d", &ch);
    printf("Enter number of frames: ");
    scanf("%d", &n);
    printf("Enter window size: ");
    scanf("%d", &w);
    printf("Enter the frame number with the problem: ");
    scanf("%d", &x);
    while(base <= n){
        end = base + w - 1;
        if(end > n) end = n;
        printf("\nWindow [%d - %d]\n", base, end);
        for(i = base; i <= end; i++)
            printf("Sender: Frame %d sent\n", i);
        if(!done && x >= base && x <= end){
            done = 1;
            for(i = base; i < x; i++)
                printf("Receiver: Frame %d received, ACK %d sent\n", i, i);
            if(ch == 3){
                printf("Receiver: Frame %d received, but ACK %d LOST\n", x, x);
                for(i = x + 1; i <= end; i++)
                    printf("Receiver: Frame %d received, ACK %d sent\n", i, i);
                if(x < end){
                    printf("Sender: ACK %d received (cumulative) -> frame %d also confirmed, no resend\n", end, x);
                    base = end + 1;
                } else {
                    printf("Sender: Timeout -> Frame %d re-sent\n", x);
                    printf("Receiver: Duplicate frame %d discarded, ACK %d sent again\n", x, x);
                    base = end + 1;
                }
                continue;
            }
            if(ch == 1) printf("Receiver: Frame %d CORRUPTED -> discarded, NAK %d sent\n", x, x);
            if(ch == 2) printf("Frame %d LOST\n", x);
            for(i = x + 1; i <= end; i++)
                printf("Receiver: Frame %d discarded (out of order)\n", i);
            printf("Sender: %s -> Go back to frame %d, resend all frames from %d\n",
                   ch == 1 ? "NAK received" : "Timeout", x, x);
            base = x;
        } else {
            for(i = base; i <= end; i++)
                printf("Receiver: Frame %d received, ACK %d sent\n", i, i);
            base = end + 1;
        }
    }
    printf("\nAll %d frames transmitted successfully.\n", n);
    return 0;
}
