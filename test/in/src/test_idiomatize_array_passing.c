#include <stdio.h>

#include "test_idiomatize_array_passing.h"

static void fill_array(int* array, int size);

void test_idiomatize_array_passing()
{
    int array0[10];
    fill_array(array0, 10);
    printf("End of test_idiomatize_array_passing().\n");
}

static void fill_array(int* array, int size)
{
    for (int i = 0; i < size; i++)
    {
        array[i] = i;
    }
}
