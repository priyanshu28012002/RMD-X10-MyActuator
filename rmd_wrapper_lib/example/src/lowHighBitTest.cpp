#include <stdio.h>
#include <stdint.h>

// Function to print binary
void print_binary(uint16_t value,int n)
{
    for (int i = n; i >= 0; i--)
    {
        printf("%d", (value >> i) & 1);
    }
}

int main()
{
    uint16_t val = 0x4B37;

    uint8_t lowByte  = val & 0x00FF;
    uint8_t highByte = (val >> 8) & 0x00FF;

    printf("Hex Value      : 0x%04X\n", val);

    printf("Binary (16bit) : ");
    print_binary(val,16);
    printf("\n");

    printf("Low Byte Hex   : 0x%02X\n", lowByte);
    printf("Low Byte Bin   : ");
    print_binary(lowByte,8);
    printf("\n");

    printf("High Byte Hex  : 0x%02X\n", highByte);
    printf("High Byte Bin  : ");
    print_binary(highByte,8);
    printf("\n");

    return 0;
}