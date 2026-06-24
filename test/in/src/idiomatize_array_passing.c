#include "idiomatize_array_passing.h"

void idiomatize_array_passing()
{
    int array0[10];
    fill_array(array0, 10);
}

void fill_array(int* array, int size)
{
    for (int i = 0; i < size; i++)
    {
        array[i] = i;
    }
}