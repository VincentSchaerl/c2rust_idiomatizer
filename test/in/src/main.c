#include <stdint.h>

#include "test_idiomatize_loops.h"
#include "test_remove_redundant_integer_casts.h"
#include "test_replace_integer_types.h"

int main()
{
    test_replace_integer_types();
    test_remove_redundant_integer_casts();
    test_idiomatize_loops();
}
