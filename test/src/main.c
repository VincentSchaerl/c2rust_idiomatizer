#include <stdint.h>

void test()
{
    int a = 2;
    uint32_t array[10];
    for (int i = 0; i < 10; i++)
    {
        array[i] = i;
    }
}

int main()
{
    return 0;
}

/*

#include <stdint.h>

void fill_arr(int* array, uint32_t size)
{
    for (int i = 0; i < size; i++)
    {
        array[i] = i;
    }
}

void test()
{
    uint32_t array[10];
    fill_arr(array, 10);
}

int main()
{
    return 0;
}

*/
