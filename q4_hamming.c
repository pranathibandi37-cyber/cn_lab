// Q4: Error detection and correction using Hamming Code (7,4)
#include <stdio.h>
int main(){
    int h[8], i, c1, c2, c4, pos;
    printf("Enter 4 data bits (space separated): ");
    scanf("%d %d %d %d", &h[3], &h[5], &h[6], &h[7]);
    h[1] = h[3] ^ h[5] ^ h[7];
    h[2] = h[3] ^ h[6] ^ h[7];
    h[4] = h[5] ^ h[6] ^ h[7];
    printf("Parity bits: P1=%d P2=%d P4=%d\n", h[1], h[2], h[4]);
    printf("Hamming code: ");
    for(i = 1; i <= 7; i++) printf("%d", h[i]);

    printf("\n\nEnter received 7 bits (space separated): ");
    for(i = 1; i <= 7; i++) scanf("%d", &h[i]);
    c1 = h[1] ^ h[3] ^ h[5] ^ h[7];
    c2 = h[2] ^ h[3] ^ h[6] ^ h[7];
    c4 = h[4] ^ h[5] ^ h[6] ^ h[7];
    pos = c4 * 4 + c2 * 2 + c1;
    if(pos == 0)
        printf("No error detected\n");
    else{
        printf("Error detected at position %d\n", pos);
        h[pos] = !h[pos];
        printf("Corrected code: ");
        for(i = 1; i <= 7; i++) printf("%d", h[i]);
        printf("\n");
    }
    return 0;
}
