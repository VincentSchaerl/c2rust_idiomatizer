#include <stdint.h>
#include <stdio.h>

#include "test_idiomatize_loops.h"
#include "nested_structs.h"

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

static int a;
static int array0[10];
static struct OuterStruct outerStruct0;
static OuterStructTypedef outerStructTypedef0;

void test_idiomatize_loops()
{
    int b;
    int array1[10];
    struct OuterStruct outerStruct1;
    OuterStructTypedef outerStructTypedef1;

    // for (<T> <I> = <L>; <I> <relational operator> <N>; <I><decrementing or incrementing>)
    // {
    //     <body>
    // }

    // base case
    
    for (int i = 0; i < 10; i++) {}

    // L

    for (int i = -1; i < 10; i++) {}
    for (int i = 0; i < 10; i++) {}
    for (int i = 1; i < 10; i++) {}

    a = 0;
    b = 0;
    for (int i = a; i < 10; i++) {}
    for (int i = b; i < 10; i++) {}

    a = 0;
    array0[0] = 0;
    array1[0] = 0;
    for (int i = array0[0]; i < 10; i++) {}
    for (int i = array0[a]; i < 10; i++) {}
    for (int i = array1[0]; i < 10; i++) {}
    for (int i = array1[a]; i < 10; i++) {}

    outerStruct0.x = 0;
    outerStruct0.innerStruct.x = 0;
    outerStructTypedef0.x = 0;
    outerStructTypedef0.innerStructTypedef.x = 0;
    outerStruct1.x = 0;
    outerStruct1.innerStruct.x = 0;
    outerStructTypedef1.x = 0;
    outerStructTypedef1.innerStructTypedef.x = 0;
    for (int i = outerStruct0.x; i < 10; i++) {}
    for (int i = outerStruct0.innerStruct.x; i < 10; i++) {}
    for (int i = outerStructTypedef0.x; i < 10; i++) {}
    for (int i = outerStructTypedef0.innerStructTypedef.x; i < 10; i++) {}
    for (int i = outerStruct1.x; i < 10; i++) {}
    for (int i = outerStruct1.innerStruct.x; i < 10; i++) {}
    for (int i = outerStructTypedef1.x; i < 10; i++) {}
    for (int i = outerStructTypedef1.innerStructTypedef.x; i < 10; i++) {}

    receive_integers_for_l(a, &a, array0, outerStruct0, &outerStruct0, outerStructTypedef0, &outerStructTypedef0);

    // <relational operator>

    for (int i = 0; i < 10; i++) {}
    for (int i = 0; i <= 10; i++) {}
    for (int i = 0; i > 10; i++) {}
    for (int i = 0; i >= 10; i++) {}

    // N

    for (int i = 0; i < -10; i++) {}
    for (int i = 0; i < 0; i++) {}
    for (int i = 0; i < 10; i++) {}

    a = 10;
    b = 10;
    for (int i = 0; i < a; i++) {}
    for (int i = 0; i < b; i++) {}

    a = 0;
    array0[0] = 10;
    array1[0] = 10;
    for (int i = 0; i < array0[0]; i++) {}
    for (int i = 0; i < array0[a]; i++) {}
    for (int i = 0; i < array1[0]; i++) {}
    for (int i = 0; i < array1[a]; i++) {}

    outerStruct0.x = 10;
    outerStruct0.innerStruct.x = 10;
    outerStructTypedef0.x = 10;
    outerStructTypedef0.innerStructTypedef.x = 10;
    outerStruct1.x = 10;
    outerStruct1.innerStruct.x = 10;
    outerStructTypedef1.x = 10;
    outerStructTypedef1.innerStructTypedef.x = 10;
    for (int i = 0; i < outerStruct0.x; i++) {}
    for (int i = 0; i < outerStruct0.innerStruct.x; i++) {}
    for (int i = 0; i < outerStructTypedef0.x; i++) {}
    for (int i = 0; i < outerStructTypedef0.innerStructTypedef.x; i++) {}
    for (int i = 0; i < outerStruct1.x; i++) {}
    for (int i = 0; i < outerStruct1.innerStruct.x; i++) {}
    for (int i = 0; i < outerStructTypedef1.x; i++) {}
    for (int i = 0; i < outerStructTypedef1.innerStructTypedef.x; i++) {}

    for (int i = 0; i < 10 + 1; i++) {}
    for (int i = 0; i < 10 - 1; i++) {}
    for (int i = 0; i < 10 * 2; i++) {}
    for (int i = 0; i < 10 / 2; i++) {}
    for (int i = 0; i < sizeof(array0) / sizeof(array0[0]); i++) {}
    for (int i = 0; i < sizeof(array1) / sizeof(array1[0]); i++) {}

    receive_integers_for_n(a, &a, array0, outerStruct0, &outerStruct0, outerStructTypedef0, &outerStructTypedef0);

    // decrementing or incrementing

    for (int i = 0; i < 10; i++) {}
    for (int i = 0; i > -10; i--) {}
    for (int i = 0; i < 10; i += 2) {}
    for (int i = 0; i > -10; i -= 2) {}
    for (int i = 0; i < 10; i = i + 3) {}
    for (int i = 0; i > -10; i = i - 3) {}

    // body

    for (int i = 0; i < 10; i++)
    {
        a = i;
        b = i;
        int c = i;
        int d;
        d = i;

        array0[0] = i;
        array1[0] = i;
        int array2[10] = {i};
        int array3[10] = {i, i, i, i, i, i, i, i, i, i};

        outerStruct0.x = i;
        outerStruct0.innerStruct.x = i;
        outerStructTypedef0.x = i;
        outerStructTypedef0.innerStructTypedef.x = i;
        outerStruct1.x = i;
        outerStruct1.innerStruct.x = i;
        outerStructTypedef1.x = i;
        outerStructTypedef1.innerStructTypedef.x = i;
        struct OuterStruct outerStruct2 = { i, { i } };
        OuterStructTypedef outerStructTypedef2 = { i, { i } };
        struct OuterStruct outerStruct3 = { .x = i, .innerStruct = { .x = i } };
        OuterStructTypedef outerStructTypedef3 = { .x = i, .innerStructTypedef = { .x = i } };

        a = array0[i];
        array0[i] = a;
        a = array1[i];
        array1[i] = a;
        struct OuterStruct outerStructs[10];
        outerStructs[i].x = a;
        outerStructs[i].innerStruct.x = a;

        a = b + i;
        a = i - b;
        a = b * i;
        b = 1;
        a = i % b;

        if (i) {};
        if (!i) {};
        if (i > 0) {};

        int* p = array0 + i;
        a = *(array0 + i);

        switch (i)
        {
            case 1:
                break;
        }

        a = (long)i;
        a = i > 0 ? i : -i;
        char str[100];
        snprintf(str, sizeof(str), "Value of i: %d\n", i);
        sizeof(i);

        receive_and_return_integers_for_body
        (
            i,
            &a,
            (int[10]){i},
            (struct OuterStruct){ i, { i } },
            &outerStruct0,
            (OuterStructTypedef){ i, { i } },
            &outerStructTypedef0
        );
        receive_and_return_integers_for_body
        (
            i, 
            &a, 
            (int[]){i, i, i, i, i, i, i, i, i, i}, 
            (struct OuterStruct){ .x = i, .innerStruct = { .x = i } },
            &outerStruct0, 
            (OuterStructTypedef){ .x = i, .innerStructTypedef = { .x = i } },
            &outerStructTypedef0
        );
    }

    // index type to usize

    for (int i = 0; i < sizeof(array0) / sizeof(array0[0]); i++)
    {
        array0[i] = 0;
    }

    // index based to iterator based

    for (int i = 0; i < sizeof(array0) / sizeof(array0[0]); i++)
    {
        array0[i] = 0;
    }

    for (int i = 0; i < sizeof(array0) / sizeof(array0[0]); i++)
    {
        array0[i] = i;
    }

    // end

    printf("End of test_idiomatize_loops().\n");
}

static void receive_integers_for_l
(
    int a,
    int* a_p,
    int array[],
    struct OuterStruct outerStruct,
    struct OuterStruct* outerStruct_p,
    OuterStructTypedef outerStructTypedef,
    OuterStructTypedef* outerStructTypedef_p
)
{
    a = 0;
    *a_p = 0;
    for (int i = a; i < 10; i++) {}
    for (int i = *a_p; i < 10; i++) {}

    a = 0;
    array[0] = 0;
    for (int i = array[0]; i < 10; i++) {}
    for (int i = array[a]; i < 10; i++) {}

    outerStruct.x = 0;
    outerStruct.innerStruct.x = 0;
    outerStruct_p->x = 0;
    outerStruct_p->innerStruct.x = 0;
    outerStructTypedef.x = 0;
    outerStructTypedef.innerStructTypedef.x = 0;
    outerStructTypedef_p->x = 0;
    outerStructTypedef_p->innerStructTypedef.x = 0;
    for (int i = outerStruct.x; i < 10; i++) {}
    for (int i = outerStruct.innerStruct.x; i < 10; i++) {}
    for (int i = outerStruct_p->x; i < 10; i++) {}
    for (int i = outerStruct_p->innerStruct.x; i < 10; i++) {}
    for (int i = outerStructTypedef.x; i < 10; i++) {}
    for (int i = outerStructTypedef.innerStructTypedef.x; i < 10; i++) {}
    for (int i = outerStructTypedef_p->x; i < 10; i++) {}
    for (int i = outerStructTypedef_p->innerStructTypedef.x; i < 10; i++) {}
}

static void receive_integers_for_n
(
    int a,
    int* a_p,
    int array[],
    struct OuterStruct outerStruct,
    struct OuterStruct* outerStruct_p,
    OuterStructTypedef outerStructTypedef,
    OuterStructTypedef* outerStructTypedef_p
)
{
    a = 10;
    *a_p = 10;
    for (int i = 0; i < a; i++) {}
    for (int i = 0; i < *a_p; i++) {}

    a = 0;
    array[0] = 10;
    for (int i = 0; i < array[0]; i++) {}
    for (int i = 0; i < array[a]; i++) {}

    outerStruct.x = 10;
    outerStruct.innerStruct.x = 10;
    outerStruct_p->x = 10;
    outerStruct_p->innerStruct.x = 10;
    outerStructTypedef.x = 10;
    outerStructTypedef.innerStructTypedef.x = 10;
    outerStruct_p->x = 10;
    outerStruct_p->innerStruct.x = 10;
    for (int i = 0; i < outerStruct.x; i++) {}
    for (int i = 0; i < outerStruct.innerStruct.x; i++) {}
    for (int i = 0; i < outerStruct_p->x; i++) {}
    for (int i = 0; i < outerStruct_p->innerStruct.x; i++) {}
    for (int i = 0; i < outerStructTypedef.x; i++) {}
    for (int i = 0; i < outerStructTypedef.innerStructTypedef.x; i++) {}
    for (int i = 0; i < outerStructTypedef_p->x; i++) {}
    for (int i = 0; i < outerStructTypedef_p->innerStructTypedef.x; i++) {}
}

static int receive_and_return_integers_for_body
(
    int a,
    int* a_p,
    int array[],
    struct OuterStruct outerStruct,
    struct OuterStruct* outerStruct_p,
    OuterStructTypedef outerStructTypedef,
    OuterStructTypedef* outerStructTypedef_p
)
{
    for (int i = 0; i < 10; i++)
    {
        a = i;
        *a_p = i;

        array[0] = i;

        outerStruct.x = i;
        outerStruct.innerStruct.x = i;
        outerStruct_p->x = i;
        outerStruct_p->innerStruct.x = i;
        outerStructTypedef.x = i;
        outerStructTypedef.innerStructTypedef.x = i;
        outerStructTypedef_p->x = i;
        outerStructTypedef_p->innerStructTypedef.x = i;

        a = array[i];
        array[i] = a;

        return i;
    }
}
