#include <stdio.h>

#include "test_idiomatize_loops_pass_arrays_as_slices_support.h"
#include "test_idiomatize_loops_pass_arrays_as_slices.h"

static void iterate_array_2
(
    int array[],
    int length
) {};

#define LENGTH_0 10

enum
{
    LENGTH_1 = 10
};

void test_idiomatize_loops_pass_arrays_as_slices_support()
{
    int array[10];
    const int array_const[10];

    iterate_array_1(array, 9);

    iterate_array_2(array, 9);

    iterate_array_6(array, 10);
    iterate_array_6(array, LENGTH_0);
    iterate_array_6(array, LENGTH_1);
    iterate_array_6(array, sizeof(array) / sizeof(array[0]));
    iterate_array_6(array_const, 10);
    iterate_array_6(array_const, LENGTH_0);
    iterate_array_6(array_const, LENGTH_1);
    iterate_array_6(array_const, sizeof(array_const) / sizeof(array_const[0]));

    printf("End of test_idiomatize_loops_pass_arrays_as_slices_support().\n");
}
