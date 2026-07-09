#pragma once

#include "nested_structs.h"

void test_idiomatize_loops();

static void receive_integers_for_l
(
    int a,
    int* a_p,
    int array[],
    struct OuterStruct outerStruct,
    struct OuterStruct* outerStruct_p,
    OuterStructTypedef outerStructTypedef,
    OuterStructTypedef* outerStructTypedef_p
);

static void receive_integers_for_n
(
    int a,
    int* a_p,
    int array[],
    struct OuterStruct outerStruct,
    struct OuterStruct* outerStruct_p,
    OuterStructTypedef outerStructTypedef,
    OuterStructTypedef* outerStructTypedef_p
);

static int receive_and_return_integers_for_body
(
    int a,
    int* a_p,
    int array[],
    struct OuterStruct outerStruct,
    struct OuterStruct* outerStruct_p,
    OuterStructTypedef outerStructTypedef,
    OuterStructTypedef* outerStructTypedef_p
);
