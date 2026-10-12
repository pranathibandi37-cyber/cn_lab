// Q3: Error detection using Cyclic Redundancy Check (CRC)
#include <stdio.h>
#include <string.h>
void divide(char t[], char g[]){
    int i, j, n = strlen(t), k = strlen(g);
    for(i = 0; i <= n - k; i++)
        if(t[i] == '1')
            for(j = 0; j < k; j++)
                t[i + j] = (t[i + j] == g[j]) ? '0' : '1';
}
int main(){
    char msg[50], gen[20], t[70], code[70];
    int i, m, k, err = 0;
    printf("Enter the binary message: ");
    scanf("%s", msg);
    printf("Enter the generator: ");
    scanf("%s", gen);
    m = strlen(msg);
    k = strlen(gen);
    strcpy(t, msg);
    for(i = 0; i < k - 1; i++) t[m + i] = '0';
    t[m + k - 1] = '\0';
    divide(t, gen);
    strcpy(code, msg);
    strcat(code, t + m);
    printf("CRC Remainder     : %s\n", t + m);
    printf("Transmitted Frame : %s\n", code);

    printf("\nEnter received frame: ");
    scanf("%s", t);
    divide(t, gen);
    printf("Receiver remainder: %s\n", t + strlen(t) - (k - 1));
    for(i = strlen(t) - (k - 1); t[i]; i++)
        if(t[i] == '1') err = 1;
    printf("%s\n", err ? "Error detected" : "No error detected");
    return 0;
}
