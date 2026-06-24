#include <stdint.h>

#include "replace_integers.h"
#include "remove_redundant_integer_casts.h"
#include "idiomatize_loops.h"
#include "idiomatize_array_passing.h"

int main()
{
    replace_integers();
    remove_redundant_integer_casts();
    idiomatize_loops();
    idiomatize_array_passing();
}
