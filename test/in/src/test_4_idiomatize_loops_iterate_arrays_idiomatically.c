#include <stdint.h>
#include <stdio.h>

#include "test_4_idiomatize_loops_iterate_arrays_idiomatically.h"

#define LENGTH_0 10

enum { LENGTH_1 = 10 };

enum { LENGTH_2 = 10 };

enum { LENGTH_3 = 2147483700 };

extern int array_global_1[10];
static int array_global_static[10];

void test_4_idiomatize_loops_iterate_arrays_idiomatically()
{
    int array[10];
    const int array_const[10];

    static int array_static[10];

    // unsuffixed integer literal expression

    for (int i_0 = 0; i_0 < 10; i_0++)
    {
        array[i_0] = i_0;
    }

    for (unsigned int i_1 = 0; i_1 < 10; i_1++)
    {
        int array_const_clone[10];
        array_const_clone[i_1] = array_const[i_1];
    }

    // LENGTH_0

    for (long long i_2 = 0; i_2 < LENGTH_0; i_2++)
    {
        array[i_2] = i_2;
    }

    for (unsigned long long i_3 = 0; i_3 < LENGTH_0; i_3++)
    {
        int array_const_clone[10];
        array_const_clone[i_3] = array_const[i_3];
    }

    // LENGTH_1

    for (uint64_t i_4 = 0; i_4 < LENGTH_1; i_4++)
    {
        array[i_4] = i_4;
    }

    for (size_t i_5 = 0; i_5 < LENGTH_1; i_5++)
    {
        int array_const_clone[10];
        array_const_clone[i_5] = array_const[i_5];
    }

    // negative cases

    for (int f_0 = 0; f_0 < 10; f_0++)
    {
        array_global_1[f_0] = f_0;
    }

    for (int f_1 = 0; f_1 < 10; f_1++)
    {
        array_global_static[f_1] = f_1;
    }

    for (int f_2 = 0; f_2 < 10; f_2++)
    {
        array_static[f_2] = f_2;
    }

    // end

    printf("End of test_4_idiomatize_loops_iterate_arrays_idiomatically().\n");
}
