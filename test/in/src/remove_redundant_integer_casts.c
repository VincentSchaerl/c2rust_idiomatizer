#include <stdint.h>

#include "remove_redundant_integer_casts.h"

int a;

int remove_redundant_integer_casts(int b)
{
    a = 0;

    b = 1;

    int c = 2;

    int d;
    d = 3;
    d = 4;

    remove_redundant_integer_casts(5);

    return 6;
}
