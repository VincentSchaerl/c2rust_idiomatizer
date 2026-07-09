#include <stdint.h>
#include <stdio.h>

#include "test_remove_redundant_integer_casts.h"
#include "nested_structs.h"

static int c;
static struct OuterStruct outerStruct0;
static OuterStructTypedef outerStructTypedef0;

void test_remove_redundant_integer_casts()
{
    int a = 0;

    int b;
    b = 1;
    b = 2;

    c = 3;

    outerStruct0.i = 4;
    outerStruct0.innerStruct.i = 5;

    struct OuterStruct outerStruct1 = { 6, { 7 } };

    struct OuterStruct outerStruct2 = { .i = 8, .innerStruct = { .i = 9 } };

    outerStructTypedef0.i = 10;
    outerStructTypedef0.innerStructTypedef.i = 11;

    OuterStructTypedef outerStructTypedef1 = { 12, { 13 } };

    OuterStructTypedef outerStructTypedef2 = { .i = 14, .innerStructTypedef = { .i = 15 } };

    receive_and_return_integers(16, (struct OuterStruct){ 17, { 18 } }, (OuterStructTypedef){ 19, { 20 } });

    receive_and_return_integers(
        21,
        (struct OuterStruct){ .i = 22, .innerStruct = { .i = 23 } },
        (OuterStructTypedef){ .i = 24, .innerStructTypedef = { .i = 25 } }
    );

    printf("End of test_remove_redundant_integer_casts().\n");
}

static int receive_and_return_integers(int d, struct OuterStruct outerStruct, OuterStructTypedef outerStructTypedef)
{
    d = 26;

    outerStruct.i = 27;
    outerStruct.innerStruct.i = 28;

    outerStructTypedef.i = 29;
    outerStructTypedef.innerStructTypedef.i = 30;

    return 31;
}
