#include "fill_array.h"

void fill_array(int* array, int size)
{
    for (int i = 0; i < size; i++)
    {
        array[i] = i;
    }
}