// Q7: Flow control using Selective Repeat ARQ
#include <stdio.h>
int main(){
    int n, w, x, i, base = 1, end, done = 0, hit;
    printf("Enter number of frames: ");
    scanf("%d", &n);
    printf("Enter window size: ");
    scanf("%d", &w);
    printf("Enter the lost frame number: ");
    scanf("%d", &x);
    while(base <= n){
        end = base + w - 1;
        if(end > n) end = n;
        hit = (!done && x >= base && x <= end);
        printf("\nWindow [%d - %d]\n", base, end);
        for(i = base; i <= end; i++){
            printf("Sender: Frame %d sent\n", i);
            if(hit && i == x)
                printf("Frame %d LOST\n", i);
            else if(hit && i > x)
                printf("Receiver: Frame %d received and BUFFERED, ACK %d sent\n", i, i);
            else
                printf("Receiver: Frame %d received, ACK %d sent\n", i, i);
        }
        if(hit){
            done = 1;
            printf("Timeout for frame %d -> Sender re-sends ONLY frame %d\n", x, x);
            printf("Receiver: Frame %d received, ACK %d sent -> frames %d to %d delivered in order\n",
                   x, x, x, end);
        }
        base = end + 1;
    }
    printf("\nAll %d frames transmitted successfully.\n", n);
    return 0;
}
