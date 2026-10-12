// Q2: Error detection using Checksum (8-bit words, 1's complement)
#include <stdio.h>
#include <stdlib.h>
void printbin(int x){
    int i;
    for(i = 7; i >= 0; i--) printf("%d", (x >> i) & 1);
}
int main(){
    int n, i, sum = 0, cs;
    char w[20];
    printf("Enter number of data words: ");
    scanf("%d", &n);
    printf("Enter %d words (8 bits each):\n", n);
    for(i = 0; i < n; i++){
        scanf("%s", w);
        sum += strtol(w, NULL, 2);
        if(sum > 255) sum = (sum & 255) + 1;    // wrap-around carry
    }
    cs = ~sum & 255;
    printf("Sum      = "); printbin(sum);
    printf("\nChecksum = "); printbin(cs);
    printf("\n");

    printf("\nReceiver: enter the %d received words:\n", n);
    sum = 0;
    for(i = 0; i < n; i++){
        scanf("%s", w);
        sum += strtol(w, NULL, 2);
        if(sum > 255) sum = (sum & 255) + 1;
    }
    sum += cs;
    if(sum > 255) sum = (sum & 255) + 1;
    printf("Receiver sum = "); printbin(sum);
    printf("\n%s\n", sum == 255 ? "No error detected" : "Error detected");
    return 0;
}
