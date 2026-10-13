#include <stdio.h>

#include "test_7_idiomatize_loops_index_type_to_usize_and_index_based_to_iterator_based.h"

void test_7_idiomatize_loops_index_type_to_usize_and_index_based_to_iterator_based ()
{
    int array[10];
    
    for (int i_0 = 0; i_0 < 10; i_0++)
    {
        array[i_0] = 0;
    }

    for (int i_1 = 0; i_1 < 10; i_1++)
    {
        array[i_1] = i_1;
    }

    for (int f_0 = 0; f_0 < 10; f_0++)
    {
        array[f_0] = 0;
        int* a = array;
    }

    printf("End of test_7_idiomatize_loops_index_type_to_usize_and_index_based_to_iterator_based().\n");
};
