// Q5: Flow control using Stop-and-Wait protocol
#include <stdio.h>
int main(){
    int n, i, lostf, losta;
    printf("Enter number of frames: ");
    scanf("%d", &n);
    printf("Enter frame number to lose (0 = none): ");
    scanf("%d", &lostf);
    printf("Enter ACK number to lose (0 = none): ");
    scanf("%d", &losta);
    for(i = 1; i <= n; i++){
        printf("\nSender: Frame %d sent\n", i);
        if(i == lostf){
            printf("Frame %d LOST -> Timeout\n", i);
            printf("Sender: Frame %d re-sent\n", i);
        }
        printf("Receiver: Frame %d received, ACK %d sent\n", i, i);
        if(i == losta){
            printf("ACK %d LOST -> Timeout\n", i);
            printf("Sender: Frame %d re-sent\n", i);
            printf("Receiver: Duplicate frame %d discarded, ACK %d sent again\n", i, i);
        }
        printf("Sender: ACK %d received\n", i);
    }
    printf("\nAll frames transmitted successfully.\n");
    return 0;
}
