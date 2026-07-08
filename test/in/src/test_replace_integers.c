#include <stdint.h>
#include <stdio.h>

#include "test_replace_integers.h"

void test_replace_integers()
{
    signed char signed_char = 2;
    unsigned char unsigned_char = 3;

    short _short = 4;
    signed short signed_short = 5;
    unsigned short unsigned_short = 6;

    int _int = 7;
    signed int signed_int = 8;
    unsigned int unsigned_int = 9;

    long _long = 10;
    signed long signed_long = 11;
    unsigned long unsigned_long = 12;

    long long _long_long = 13;
    signed long long signed_long_long = 14;
    unsigned long long unsigned_long_long = 15;

    int8_t _int8_t = 16;
    uint8_t _uint8_t = 17;

    int16_t _int16_t = 18;
    uint16_t _uint16_t = 19;
    
    int32_t _int32_t = 20;
    uint32_t _uint32_t = 21;
    
    int64_t _int64_t = 22;
    uint64_t _uint64_t = 23;

    printf("End of test_replace_integers().\n");
}
