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

    outerStruct0.x = 4;
    outerStruct0.innerStruct.x = 5;

    struct OuterStruct outerStruct1 = { 6, { 7 } };

    struct OuterStruct outerStruct2 = { .x = 8, .innerStruct = { .x = 9 } };

    outerStructTypedef0.x = 10;
    outerStructTypedef0.innerStructTypedef.x = 11;

    OuterStructTypedef outerStructTypedef1 = { 12, { 13 } };

    OuterStructTypedef outerStructTypedef2 = { .x = 14, .innerStructTypedef = { .x = 15 } };

    receive_and_return_integers(16, (struct OuterStruct){ 17, { 18 } }, (OuterStructTypedef){ 19, { 20 } });

    receive_and_return_integers(
        21,
        (struct OuterStruct){ .x = 22, .innerStruct = { .x = 23 } },
        (OuterStructTypedef){ .x = 24, .innerStructTypedef = { .x = 25 } }
    );

    printf("End of test_remove_redundant_integer_casts().\n");
}

static int receive_and_return_integers(int d, struct OuterStruct outerStruct, OuterStructTypedef outerStructTypedef)
{
    d = 26;

    outerStruct.x = 27;
    outerStruct.innerStruct.x = 28;

    outerStructTypedef.x = 29;
    outerStructTypedef.innerStructTypedef.x = 30;

    return 31;
}
