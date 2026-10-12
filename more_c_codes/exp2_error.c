/* EX.NO 2 - Error Detection and Correction
   1. Single parity  2. Two-dimensional parity  3. CRC  4. Checksum  5. Hamming code */
#include <stdio.h>
#include <string.h>

void single_parity()
{
    char d[100];
    int i, ones = 0;
    printf("Enter data bits: ");
    scanf("%s", d);
    for (i = 0; d[i]; i++)
        if (d[i] == '1') ones++;
    printf("Number of 1s     : %d\n", ones);
    printf("Even parity bit  : %d  -> Transmitted: %s%d\n", ones % 2, d, ones % 2);
    printf("Odd parity bit   : %d  -> Transmitted: %s%d\n", 1 - ones % 2, d, 1 - ones % 2);

    printf("\nEnter received bits (even parity): ");
    scanf("%s", d);
    ones = 0;
    for (i = 0; d[i]; i++)
        if (d[i] == '1') ones++;
    printf(ones % 2 == 0 ? "RESULT: No error detected\n" : "RESULT: Error detected\n");
}

void two_d_parity()
{
    int a[10][10], r, c, i, j, sum;
    printf("Enter number of rows and columns: ");
    scanf("%d %d", &r, &c);
    printf("Enter the data bits row by row:\n");
    for (i = 0; i < r; i++)
        for (j = 0; j < c; j++)
            scanf("%d", &a[i][j]);
    for (i = 0; i < r; i++) {                 /* row parity */
        sum = 0;
        for (j = 0; j < c; j++) sum += a[i][j];
        a[i][c] = sum % 2;
    }
    for (j = 0; j <= c; j++) {                /* column parity (incl. parity column) */
        sum = 0;
        for (i = 0; i < r; i++) sum += a[i][j];
        a[r][j] = sum % 2;
    }
    printf("\nData with row and column parity (even):\n");
    for (i = 0; i <= r; i++) {
        for (j = 0; j <= c; j++) {
            if (j == c) printf("| ");
            printf("%d ", a[i][j]);
        }
        printf("\n");
        if (i == r - 1) {
            for (j = 0; j <= c; j++) printf("--");
            printf("\n");
        }
    }
    printf("RESULT: Parity bits generated successfully\n");
}

/* XOR long division: remainder ends up in the last (len(g)-1) chars of t */
void xor_divide(char *t, char *g)
{
    int n = strlen(t), gl = strlen(g), i, j;
    for (i = 0; i <= n - gl; i++)
        if (t[i] == '1')
            for (j = 0; j < gl; j++)
                t[i + j] = (t[i + j] == g[j]) ? '0' : '1';
}

void crc()
{
    char data[100], gen[30], t[130], rem[30], code[130];
    int dl, gl, i;
    printf("Enter data bits     : ");
    scanf("%s", data);
    printf("Enter generator bits: ");
    scanf("%s", gen);
    dl = strlen(data);
    gl = strlen(gen);

    strcpy(t, data);
    for (i = 0; i < gl - 1; i++) t[dl + i] = '0';
    t[dl + gl - 1] = '\0';
    printf("Data after appending %d zeros: %s\n", gl - 1, t);

    xor_divide(t, gen);
    strcpy(rem, t + dl);
    strcpy(code, data);
    strcat(code, rem);
    printf("CRC remainder : %s\n", rem);
    printf("Codeword sent : %s\n", code);

    printf("\nEnter received codeword: ");
    scanf("%s", t);
    xor_divide(t, gen);
    printf("Remainder at receiver: %s\n", t + strlen(t) - (gl - 1));
    for (i = strlen(t) - (gl - 1); t[i]; i++)
        if (t[i] == '1') {
            printf("RESULT: Error detected\n");
            return;
        }
    printf("RESULT: No error detected\n");
}

int bin_to_int(char *s)
{
    int v = 0;
    while (*s) v = v * 2 + (*s++ - '0');
    return v;
}

void print_bin(int v, int bits)
{
    int i;
    for (i = bits - 1; i >= 0; i--) printf("%d", (v >> i) & 1);
}

int wrap_sum(int *w, int k, int bits)
{
    int i, sum = 0, mask = (1 << bits) - 1;
    for (i = 0; i < k; i++) {
        sum += w[i];
        while (sum > mask) sum = (sum & mask) + (sum >> bits);   /* add carry back */
    }
    return sum;
}

void checksum()
{
    char s[40];
    int w[20], k, bits = 0, i, sum, cs, mask;
    printf("Enter number of data words: ");
    scanf("%d", &k);
    for (i = 0; i < k; i++) {
        printf("Word %d: ", i + 1);
        scanf("%s", s);
        bits = strlen(s);
        w[i] = bin_to_int(s);
    }
    mask = (1 << bits) - 1;
    sum = wrap_sum(w, k, bits);
    cs = ~sum & mask;
    printf("Sum (with wrap-around): ");
    print_bin(sum, bits);
    printf("\nChecksum (1's complement): ");
    print_bin(cs, bits);
    printf("\n");

    printf("\nReceiver - enter the %d received words:\n", k);
    for (i = 0; i < k; i++) {
        printf("Word %d: ", i + 1);
        scanf("%s", s);
        w[i] = bin_to_int(s);
    }
    w[k] = cs;
    sum = wrap_sum(w, k + 1, bits);
    printf("Receiver sum: ");
    print_bin(sum, bits);
    printf("  -> complement: ");
    print_bin(~sum & mask, bits);
    printf(((~sum & mask) == 0) ? "\nRESULT: No error detected\n" : "\nRESULT: Error detected\n");
}

void hamming()
{
    char d[10];
    int h[8], i, c1, c2, c4, pos;
    printf("Enter 4 data bits: ");
    scanf("%s", d);
    h[3] = d[0] - '0';
    h[5] = d[1] - '0';
    h[6] = d[2] - '0';
    h[7] = d[3] - '0';
    h[1] = h[3] ^ h[5] ^ h[7];
    h[2] = h[3] ^ h[6] ^ h[7];
    h[4] = h[5] ^ h[6] ^ h[7];
    printf("P1=%d P2=%d P4=%d\n", h[1], h[2], h[4]);
    printf("Hamming codeword (positions 1-7): ");
    for (i = 1; i <= 7; i++) printf("%d", h[i]);

    printf("\n\nEnter received 7-bit codeword: ");
    scanf("%s", d);
    for (i = 1; i <= 7; i++) h[i] = d[i - 1] - '0';
    c1 = h[1] ^ h[3] ^ h[5] ^ h[7];
    c2 = h[2] ^ h[3] ^ h[6] ^ h[7];
    c4 = h[4] ^ h[5] ^ h[6] ^ h[7];
    pos = c4 * 4 + c2 * 2 + c1;
    printf("Syndrome (c4 c2 c1) = %d%d%d = %d\n", c4, c2, c1, pos);
    if (pos == 0) {
        printf("RESULT: No error detected\n");
    } else {
        h[pos] ^= 1;
        printf("Error at bit position %d\nCorrected codeword: ", pos);
        for (i = 1; i <= 7; i++) printf("%d", h[i]);
        printf("\nRESULT: Error corrected\n");
    }
}

int main()
{
    int ch;
    printf("1.Single Parity  2.Two-D Parity  3.CRC  4.Checksum  5.Hamming Code\nEnter choice: ");
    scanf("%d", &ch);
    switch (ch) {
    case 1: single_parity(); break;
    case 2: two_d_parity(); break;
    case 3: crc(); break;
    case 4: checksum(); break;
    case 5: hamming(); break;
    default: printf("Invalid choice\n");
    }
    return 0;
}
