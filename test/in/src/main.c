#include <stdint.h>

#include "test_0_replace_integer_types.h"
#include "test_1_remove_redundant_integer_casts.h"
#include "test_3_idiomatize_loops_initial_transformation.h"
#include "test_4_idiomatize_loops_iterate_arrays_idiomatically.h"
#include "test_5_idiomatize_loops_pass_arrays_as_slices.h"
#include "test_6_idiomatize_loops_pass_arrays_as_slices_support.h"

int main()
{
    test_0_replace_integer_types();
    test_1_remove_redundant_integer_casts();
    test_3_idiomatize_loops_initial_transformation();
    test_4_idiomatize_loops_iterate_arrays_idiomatically();
    test_5_idiomatize_loops_pass_arrays_as_slices();
    test_6_idiomatize_loops_pass_arrays_as_slices_support();
}
