#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    char hex[17];
    unsigned long decimal;

    char address_s[17] = "0x5e9590";
    unsigned long address = strtoul(address_s, NULL, 16);
    printf("Start address: %s = %lu\n\n", address_s, address);
    while (1) {
        printf("hex: ");
        scanf("%16s", hex);
        decimal = strtoul(hex, NULL, 16);
        printf("dec: %lu\n\n", decimal - address);
    }
    return 0;
}
