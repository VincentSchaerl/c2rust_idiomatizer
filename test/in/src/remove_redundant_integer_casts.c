#include <stdint.h>

#include "remove_redundant_integer_casts.h"

struct InnerStruct
{
    int i;
};

struct OuterStruct
{
    int i;
    struct InnerStruct innerStruct;
};

typedef struct
{
    int i;
} InnerStructTypedef;

typedef struct
{
    int i;
    InnerStructTypedef innerStructTypedef;
} OuterStructTypedef;

struct OuterStruct outerStruct0;

OuterStructTypedef outerStructTypedef0;

int c;

int remove_redundant_integer_casts(int d, struct OuterStruct outerStruct1, OuterStructTypedef outerStructTypedef1)
{
    int a = 0;

    int b;
    b = 1;
    b = 2;

    c = 3;

    d = 4;

    outerStruct0.i = 5;
    outerStruct0.innerStruct.i = 6;

    outerStruct1.i = 7;
    outerStruct1.innerStruct.i = 8;

    struct OuterStruct outerStruct2 = { 9, { 10 } };

    struct OuterStruct outerStruct3 = { .i = 11, .innerStruct = { .i = 12 } };

    outerStructTypedef0.i = 13;
    outerStructTypedef0.innerStructTypedef.i = 14;

    outerStructTypedef1.i = 15;
    outerStructTypedef1.innerStructTypedef.i = 16;

    OuterStructTypedef outerStructTypedef2 = { 17, { 18 } };

    OuterStructTypedef outerStructTypedef3 = { .i = 19, .innerStructTypedef = { .i = 20 } };

    remove_redundant_integer_casts(21, (struct OuterStruct){ 22, { 23 } }, (OuterStructTypedef){ 24, { 25 } });

    remove_redundant_integer_casts(
        26,
        (struct OuterStruct){ .i = 27, .innerStruct = { .i = 28 } },
        (OuterStructTypedef){ .i = 29, .innerStructTypedef = { .i = 30 } }
    );

    return 31;
}
