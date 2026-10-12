// Q1: Error detection using even and odd parity check
#include <stdio.h>
int main(){
    char data[50], recv[51];
    int i, ones = 0, even, odd;
    printf("Enter binary data: ");
    scanf("%s", data);
    for(i = 0; data[i]; i++)
        if(data[i] == '1') ones++;
    even = ones % 2;
    odd = !even;
    printf("Number of 1s = %d\n", ones);
    printf("Even parity bit = %d -> Transmitted: %s%d\n", even, data, even);
    printf("Odd parity bit  = %d -> Transmitted: %s%d\n", odd, data, odd);

    printf("\nEnter received data (with parity bit): ");
    scanf("%s", recv);
    ones = 0;
    for(i = 0; recv[i]; i++)
        if(recv[i] == '1') ones++;
    printf("Even parity check: %s\n", ones % 2 == 0 ? "No error detected" : "Error detected");
    printf("Odd parity check : %s\n", ones % 2 == 1 ? "No error detected" : "Error detected");
    return 0;
}
