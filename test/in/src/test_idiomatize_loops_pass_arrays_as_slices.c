#include <stdint.h>
#include <stdio.h>

#include "test_idiomatize_loops_pass_arrays_as_slices.h"
#include "nested_struct.h"

static void iterate_array_0
(
    int array[],
    int length
);

static void iterate_array_2
(
    int array[],
    int length
);

static int iterate_array_3
(
    int arg0,
    size_t length,
    const int array[],
    char arg1
);

static outer_struct_t iterate_array_4
(
    int array[const],
    outer_struct_t arg0,
    const unsigned char length
);

static outer_struct_t* iterate_array_5
(
    int arg0,
    const int64_t length,
    char arg1,
    const int array[const],
    outer_struct_t* arg2
);


#define LENGTH_0 10

enum
{
    LENGTH_1 = 10
};

void test_idiomatize_loops_pass_arrays_as_slices()
{
    int array[10];
    const int array_const[10];
    char _char;
    int _int;
    outer_struct_t outer_struct;
    outer_struct_t* outer_struct_ptr;

    iterate_array_0(array, 9);

    iterate_array_1(array, 10);

    iterate_array_2(array, 10);
    iterate_array_2(array, LENGTH_0);
    iterate_array_2(array, LENGTH_1);
    iterate_array_2(array, sizeof(array) / sizeof(array[0]));

    iterate_array_3(_int, 10, array, _char);
    iterate_array_3(_int, LENGTH_0, array, _char);
    iterate_array_3(_int, LENGTH_1, array, _char);
    iterate_array_3(_int, sizeof(array) / sizeof(array[0]), array, _char);
    iterate_array_3(_int, 10, array_const, _char);
    iterate_array_3(_int, LENGTH_0, array_const, _char);
    iterate_array_3(_int, LENGTH_1, array_const, _char);
    iterate_array_3(_int, sizeof(array_const) / sizeof(array_const[0]), array_const, _char);

    iterate_array_4(array, outer_struct, 10);
    iterate_array_4(array, outer_struct, LENGTH_0);
    iterate_array_4(array, outer_struct, LENGTH_1);
    iterate_array_4(array, outer_struct, sizeof(array) / sizeof(array[0]));

    iterate_array_5(_int, 10, _char, array, outer_struct_ptr);
    iterate_array_5(_int, LENGTH_0, _char, array, outer_struct_ptr);
    iterate_array_5(_int, LENGTH_1, _char, array, outer_struct_ptr);
    iterate_array_5(_int, sizeof(array) / sizeof(array[0]), _char, array, outer_struct_ptr);
    iterate_array_5(_int, 10, _char, array_const, outer_struct_ptr);
    iterate_array_5(_int, LENGTH_0, _char, array_const, outer_struct_ptr);
    iterate_array_5(_int, LENGTH_1, _char, array_const, outer_struct_ptr);
    iterate_array_5(_int, sizeof(array_const) / sizeof(array_const[0]), _char, array_const, outer_struct_ptr);

    iterate_array_6(array, 10);
    iterate_array_6(array, LENGTH_0);
    iterate_array_6(array, LENGTH_1);
    iterate_array_6(array, sizeof(array) / sizeof(array[0]));
    iterate_array_6(array_const, 10);
    iterate_array_6(array_const, LENGTH_0);
    iterate_array_6(array_const, LENGTH_1);
    iterate_array_6(array_const, sizeof(array_const) / sizeof(array_const[0]));

    printf("End of test_idiomatize_loops_pass_arrays_as_slices().\n");
}

static void iterate_array_0
(
    int array[],
    int length
)
{
    for (int f_0 = 0; f_0 < length; f_0++)
    {
        array[f_0] = f_0;
    }
}


void iterate_array_1
(
    int array[],
    int length
)
{
    for (int f_1 = 0; f_1 < length; f_1++)
    {
        array[f_1] = f_1;
    }
}

static void iterate_array_2
(
    int array[],
    int length
)
{
    for (int i_0 = 0; i_0 < length; i_0++)
    {
        array[i_0] = i_0;
    }
    for (int i_1 = 0; i_1 < length; i_1++)
    {
        array[i_1] = i_1;
    }
}

static int iterate_array_3
(
    int arg0,
    size_t length,
    const int array[],
    char arg1
)
{
    int array_clone[length];
    for (size_t i_2 = 0; i_2 < length; i_2++)
    {
        array_clone[i_2] = array[i_2];
    }
    for (size_t i_3 = 0; i_3 < length; i_3++)
    {
        array_clone[i_3] = array[i_3];
    }
    return 0;
}

static outer_struct_t iterate_array_4
(
    int array[const],
    outer_struct_t arg0,
    const unsigned char length
)
{
    for (int i_4 = 0; i_4 < length; i_4++)
    {
        array[i_4] = i_4;
    }
    for (int i_5 = 0; i_5 < length; i_5++)
    {
        array[i_5] = i_5;
    }
    return arg0;
}

static outer_struct_t* iterate_array_5
(
    int arg0,
    const int64_t length,
    char arg1,
    const int array[const],
    outer_struct_t* arg2
)
{
    int array_clone[length];
    for (int64_t i_6 = 0; i_6 < length; i_6++)
    {
        array_clone[i_6] = array[i_6];
    }
    for (int64_t i_7 = 0; i_7 < length; i_7++)
    {
        array_clone[i_7] = array[i_7];
    }
    return arg2;
}

void iterate_array_6
(
    const int array[],
    int length
)
{
    int array_clone[length];
    for (int i_8 = 0; i_8 < length; i_8++)
    {
        array_clone[i_8] = array[i_8];
    }
    for (int i_9 = 0; i_9 < length; i_9++)
    {
        array_clone[i_9] = array[i_9];
    }
}
