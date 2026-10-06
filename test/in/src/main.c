#include <stdint.h>

#include "test_idiomatize_loops_initial_transformation.h"
#include "test_idiomatize_loops_pass_arrays_as_slices.h"
#include "test_idiomatize_loops_pass_arrays_as_slices_support.h"
#include "test_remove_redundant_integer_casts.h"
#include "test_replace_integer_types.h"

int main()
{
    test_replace_integer_types();
    test_remove_redundant_integer_casts();
    test_idiomatize_loops_initial_transformation();
    test_idiomatize_loops_pass_arrays_as_slices();
    test_idiomatize_loops_pass_arrays_as_slices_support();
}
