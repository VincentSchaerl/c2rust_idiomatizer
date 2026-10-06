#include <stdint.h>
#include <stdio.h>

#include "test_0_replace_integer_types.h"

void test_0_replace_integer_types()
{
    signed char _signed_char = 0;
    unsigned char _unsigned_char = 1;

    short _short = 2;
    unsigned short _unsigned_short = 4;

    int _int = 5;
    unsigned int _unsigned_int = 7;

    long _long = 8;
    unsigned long _unsigned_long = 10;

    long long _long_long = 11;
    unsigned long long _unsigned_long_long = 13;

    int8_t _int8_t = 14;
    uint8_t _uint8_t = 15;

    int16_t _int16_t = 16;
    uint16_t _uint16_t = 17;
    
    int32_t _int32_t = 18;
    uint32_t _uint32_t = 19;
    
    int64_t _int64_t = 20;
    uint64_t _uint64_t = 21;

    intptr_t _intptr_t = 22;
    uintptr_t _uintptr__t = 23;
    size_t _size__t = 24;

    printf("End of test_0_replace_integer_types().\n");
}
