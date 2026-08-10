#include <stdint.h>

#include "test_idiomatize_loops.h"
#include "test_remove_redundant_integer_casts.h"
#include "test_replace_integers.h"

int main()
{
    test_replace_integers();
    test_remove_redundant_integer_casts();
    test_idiomatize_loops();
}
