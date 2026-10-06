#include "stdint.h"
#include "stdio.h"

#include "test_idiomatize_loops_pass_arrays_as_slices.h"
#include "nested_struct.h"

static void iterate_array_0
(
    int array[],
    int length_int
);

static int iterate_array_1
(
    int arg0,
    size_t length_size_t,
    const int array_const_0[],
    char arg1
);

static outer_struct_t iterate_array_2
(
    int array_const_1[const],
    outer_struct_t arg0,
    const unsigned char length_unsigned_char_const
);

static outer_struct_t* iterate_array_3
(
    int arg0,
    const int64_t length_int64_t_const,
    char arg1,
    const int array_const_2[const],
    outer_struct_t* arg2
);

static void iterate_array_4
(
    int array[],
    int length
);

#define LENGTH_0 10

enum
{
    LENGTH_1 = 10
};

extern int array_global[10];

void test_idiomatize_loops_pass_arrays_as_slices()
{
    int array[10];
    const int array_const[10];
    char _char;
    int _int;
    outer_struct_t outer_struct;
    outer_struct_t* outer_struct_ptr;

    iterate_array_0(array_global, 10);
    iterate_array_0(array_global, LENGTH_0);
    iterate_array_0(array_global, LENGTH_1);
    iterate_array_0(array_global, sizeof(array_global) / sizeof(array_global[0]));
    iterate_array_0(array, 10);
    iterate_array_0(array, LENGTH_0);
    iterate_array_0(array, LENGTH_1);
    iterate_array_0(array, sizeof(array) / sizeof(array[0]));

    iterate_array_1(_int, 10, array_global, _char);
    iterate_array_1(_int, LENGTH_0, array_global, _char);
    iterate_array_1(_int, LENGTH_1, array_global, _char);
    iterate_array_1(_int, sizeof(array_global) / sizeof(array_global[0]), array_global, _char);
    iterate_array_1(_int, 10, array, _char);
    iterate_array_1(_int, LENGTH_0, array, _char);
    iterate_array_1(_int, LENGTH_1, array, _char);
    iterate_array_1(_int, sizeof(array) / sizeof(array[0]), array, _char);
    iterate_array_1(_int, 10, array_const, _char);
    iterate_array_1(_int, LENGTH_0, array_const, _char);
    iterate_array_1(_int, LENGTH_1, array_const, _char);
    iterate_array_1(_int, sizeof(array_const) / sizeof(array_const[0]), array_const, _char);

    iterate_array_2(array_global, outer_struct, 10);
    iterate_array_2(array_global, outer_struct, LENGTH_0);
    iterate_array_2(array_global, outer_struct, LENGTH_1);
    iterate_array_2(array_global, outer_struct, sizeof(array_global) / sizeof(array_global[0]));
    iterate_array_2(array, outer_struct, 10);
    iterate_array_2(array, outer_struct, LENGTH_0);
    iterate_array_2(array, outer_struct, LENGTH_1);
    iterate_array_2(array, outer_struct, sizeof(array) / sizeof(array[0]));

    iterate_array_3(_int, 10, _char, array_global, outer_struct_ptr);
    iterate_array_3(_int, LENGTH_0, _char, array_global, outer_struct_ptr);
    iterate_array_3(_int, LENGTH_1, _char, array_global, outer_struct_ptr);
    iterate_array_3(_int, sizeof(array_global) / sizeof(array_global[0]), _char, array_global, outer_struct_ptr);
    iterate_array_3(_int, 10, _char, array, outer_struct_ptr);
    iterate_array_3(_int, LENGTH_0, _char, array, outer_struct_ptr);
    iterate_array_3(_int, LENGTH_1, _char, array, outer_struct_ptr);
    iterate_array_3(_int, sizeof(array) / sizeof(array[0]), _char, array, outer_struct_ptr);
    iterate_array_3(_int, 10, _char, array_const, outer_struct_ptr);
    iterate_array_3(_int, LENGTH_0, _char, array_const, outer_struct_ptr);
    iterate_array_3(_int, LENGTH_1, _char, array_const, outer_struct_ptr);
    iterate_array_3(_int, sizeof(array_const) / sizeof(array_const[0]), _char, array_const, outer_struct_ptr);

    iterate_array_4(array, 9);

    printf("End of test_idiomatize_loops_pass_arrays_as_slices().\n");
}

static void iterate_array_0
(
    int array[],
    int length_int
)
{
    for (int i_0 = 0; i_0 < length_int; i_0++)
    {
        array[i_0] = i_0;
    }
    for (int i_1 = 0; i_1 < length_int; i_1++)
    {
        array[i_1] = i_1;
    }
}

static int iterate_array_1
(
    int arg0,
    size_t length_size_t,
    const int array_const_0[],
    char arg1
)
{
    int array_const_0_clone[length_size_t];
    for (size_t i_2 = 0; i_2 < length_size_t; i_2++)
    {
        array_const_0_clone[i_2] = array_const_0[i_2];
    }
    for (size_t i_3 = 0; i_3 < length_size_t; i_3++)
    {
        array_const_0_clone[i_3] = array_const_0[i_3];
    }
    return 0;
}

static outer_struct_t iterate_array_2
(
    int array_const_1[const],
    outer_struct_t arg0,
    const unsigned char length_unsigned_char_const
)
{
    for (int i_4 = 0; i_4 < length_unsigned_char_const; i_4++)
    {
        array_const_1[i_4] = i_4;
    }
    for (int i_5 = 0; i_5 < length_unsigned_char_const; i_5++)
    {
        array_const_1[i_5] = i_5;
    }
    return arg0;
}

static outer_struct_t* iterate_array_3
(
    int arg0,
    const int64_t length_int64_t_const,
    char arg1,
    const int array_const_2[const],
    outer_struct_t* arg2
)
{
    int array_const_2_clone[length_int64_t_const];
    for (int64_t i_6 = 0; i_6 < length_int64_t_const; i_6++)
    {
        array_const_2_clone[i_6] = array_const_2[i_6];
    }
    for (int64_t i_7 = 0; i_7 < length_int64_t_const; i_7++)
    {
        array_const_2_clone[i_7] = array_const_2[i_7];
    }
    return arg2;
}

static void iterate_array_4
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
