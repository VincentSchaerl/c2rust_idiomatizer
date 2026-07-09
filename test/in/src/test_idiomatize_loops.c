#include <stdio.h>

#include "test_idiomatize_loops.h"
#include "nested_structs.h"

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
    for (int i = array0[0]; i < 10; i++) {}
    for (int i = array0[a]; i < 10; i++) {}
    for (int i = array1[0]; i < 10; i++) {}
    for (int i = array1[a]; i < 10; i++) {}

    outerStruct0.i = 0;
    outerStruct0.innerStruct.i = 0;
    outerStructTypedef0.i = 0;
    outerStructTypedef0.innerStructTypedef.i = 0;
    outerStruct1.i = 0;
    outerStruct1.innerStruct.i = 0;
    outerStructTypedef1.i = 0;
    outerStructTypedef1.innerStructTypedef.i = 0;
    for (int i = outerStruct0.i; i < 10; i++) {}
    for (int i = outerStruct0.innerStruct.i; i < 10; i++) {}
    for (int i = outerStructTypedef0.i; i < 10; i++) {}
    for (int i = outerStructTypedef0.innerStructTypedef.i; i < 10; i++) {}
    for (int i = outerStruct1.i; i < 10; i++) {}
    for (int i = outerStruct1.innerStruct.i; i < 10; i++) {}
    for (int i = outerStructTypedef1.i; i < 10; i++) {}
    for (int i = outerStructTypedef1.innerStructTypedef.i; i < 10; i++) {}

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

    outerStruct0.i = 10;
    outerStruct0.innerStruct.i = 10;
    outerStructTypedef0.i = 10;
    outerStructTypedef0.innerStructTypedef.i = 10;
    outerStruct1.i = 10;
    outerStruct1.innerStruct.i = 10;
    outerStructTypedef1.i = 10;
    outerStructTypedef1.innerStructTypedef.i = 10;
    for (int i = 0; i < outerStruct0.i; i++) {}
    for (int i = 0; i < outerStruct0.innerStruct.i; i++) {}
    for (int i = 0; i < outerStructTypedef0.i; i++) {}
    for (int i = 0; i < outerStructTypedef0.innerStructTypedef.i; i++) {}
    for (int i = 0; i < outerStruct1.i; i++) {}
    for (int i = 0; i < outerStruct1.innerStruct.i; i++) {}
    for (int i = 0; i < outerStructTypedef1.i; i++) {}
    for (int i = 0; i < outerStructTypedef1.innerStructTypedef.i; i++) {}

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

        outerStruct0.i = i;
        outerStruct0.innerStruct.i = i;
        outerStructTypedef0.i = i;
        outerStructTypedef0.innerStructTypedef.i = i;
        outerStruct1.i = i;
        outerStruct1.innerStruct.i = i;
        outerStructTypedef1.i = i;
        outerStructTypedef1.innerStructTypedef.i = i;
        struct OuterStruct outerStruct2 = { i, { i } };
        OuterStructTypedef outerStructTypedef2 = { i, { i } };
        struct OuterStruct outerStruct3 = { .i = i, .innerStruct = { .i = i } };
        OuterStructTypedef outerStructTypedef3 = { .i = i, .innerStructTypedef = { .i = i } };

        a = array0[i];
        array0[i] = a;
        a = array1[i];
        array1[i] = a;
        struct OuterStruct outerStructs[10];
        outerStructs[i].i = a;
        outerStructs[i].innerStruct.i = a;

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
            (struct OuterStruct){ .i = i, .innerStruct = { .i = i } },
            &outerStruct0, 
            (OuterStructTypedef){ .i = i, .innerStructTypedef = { .i = i } },
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

    outerStruct.i = 0;
    outerStruct.innerStruct.i = 0;
    outerStruct_p->i = 0;
    outerStruct_p->innerStruct.i = 0;
    outerStructTypedef.i = 0;
    outerStructTypedef.innerStructTypedef.i = 0;
    outerStructTypedef_p->i = 0;
    outerStructTypedef_p->innerStructTypedef.i = 0;
    for (int i = outerStruct.i; i < 10; i++) {}
    for (int i = outerStruct.innerStruct.i; i < 10; i++) {}
    for (int i = outerStruct_p->i; i < 10; i++) {}
    for (int i = outerStruct_p->innerStruct.i; i < 10; i++) {}
    for (int i = outerStructTypedef.i; i < 10; i++) {}
    for (int i = outerStructTypedef.innerStructTypedef.i; i < 10; i++) {}
    for (int i = outerStructTypedef_p->i; i < 10; i++) {}
    for (int i = outerStructTypedef_p->innerStructTypedef.i; i < 10; i++) {}
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

    outerStruct.i = 10;
    outerStruct.innerStruct.i = 10;
    outerStruct_p->i = 10;
    outerStruct_p->innerStruct.i = 10;
    outerStructTypedef.i = 10;
    outerStructTypedef.innerStructTypedef.i = 10;
    outerStruct_p->i = 10;
    outerStruct_p->innerStruct.i = 10;
    for (int i = 0; i < outerStruct.i; i++) {}
    for (int i = 0; i < outerStruct.innerStruct.i; i++) {}
    for (int i = 0; i < outerStruct_p->i; i++) {}
    for (int i = 0; i < outerStruct_p->innerStruct.i; i++) {}
    for (int i = 0; i < outerStructTypedef.i; i++) {}
    for (int i = 0; i < outerStructTypedef.innerStructTypedef.i; i++) {}
    for (int i = 0; i < outerStructTypedef_p->i; i++) {}
    for (int i = 0; i < outerStructTypedef_p->innerStructTypedef.i; i++) {}
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

        outerStruct.i = i;
        outerStruct.innerStruct.i = i;
        outerStruct_p->i = i;
        outerStruct_p->innerStruct.i = i;
        outerStructTypedef.i = i;
        outerStructTypedef.innerStructTypedef.i = i;
        outerStructTypedef_p->i = i;
        outerStructTypedef_p->innerStructTypedef.i = i;

        a = array[i];
        array[i] = a;

        return i;
    }
}
