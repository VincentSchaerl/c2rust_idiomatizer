#include <stdint.h>
#include <stdio.h>

#include "test_replace_integer_types.h"

void test_replace_integer_types()
{
    signed char signed_char = 0;
    unsigned char unsigned_char = 1;

    short _short = 2;
    signed short signed_short = 3;
    unsigned short unsigned_short = 4;

    int _int = 5;
    signed int signed_int = 6;
    unsigned int unsigned_int = 7;

    long _long = 8;
    signed long signed_long = 9;
    unsigned long unsigned_long = 10;

    long long _long_long = 11;
    signed long long signed_long_long = 12;
    unsigned long long unsigned_long_long = 13;

    int8_t _int8_t = 14;
    uint8_t _uint8_t = 15;

    int16_t _int16_t = 16;
    uint16_t _uint16_t = 17;
    
    int32_t _int32_t = 18;
    uint32_t _uint32_t = 19;
    
    int64_t _int64_t = 20;
    uint64_t _uint64_t = 21;

    intptr_t intptr___t = 22;
    uintptr_t uintptr___t = 23;
    size_t size___t = 24;

    printf("End of test_replace_integer_types().\n");
}
