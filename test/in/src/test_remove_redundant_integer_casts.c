#include <stdint.h>
#include <stdio.h>

#include "test_remove_redundant_integer_casts.h"
#include "test_remove_redunant_integer_casts_support.h"
#include "nested_struct.h"

static void keep_variable_global_const(int variable_global_const);
static void assign_to_variable(int variable);
static void compare_with_variable_value(int variable, const int variable_const);
static void assign_to_memory_location(int* pointer, int* const pointer_const_1, int** pointer_2d);
static void compare_with_memory_location_value
(
    int* pointer,
    const int* pointer_const_0,
    int* const pointer_const_1,
    int** pointer_2d
);
static void assign_to_struct_member(outer_struct_t outer_struct);
static void compare_with_struct_member_value
(
    outer_struct_t outer_struct,
    const outer_struct_t outer_struct_const
);
static void assign_to_member_of_struct_dereferenced_via_pointer
(
    outer_struct_t* outer_struct_ptr,
    outer_struct_t* const outer_struct_ptr_const_1
);
static void compare_with_member_value_of_struct_dereferenced_via_pointer
(
    outer_struct_t* outer_struct_ptr,
    const outer_struct_t* outer_struct_ptr_const_0,
    outer_struct_t* const outer_struct_ptr_const_1
);
static void receive_integer(int a);
static int return_integer();

// Initialization of a variable
int variable_global = 0;
#define VARIABLE_GLOBAL_CONST (-1)
static int variable_global_static = 2;

// Initialization of an array
int array_global[4] = {-3, 4, -5, 6};
static int array_global_static[4] = {-7, 8, -9, 10};
int array_2d_global[2][2] = {{-11, 12}, {-13, 14}};

// Initialization of a struct
outer_struct_t outer_struct_global = { -15, { 16 } };
static outer_struct_t outer_struct_global_static = { -17, { 18 } };

int* pointer_global = &variable_global;
const int* pointer_global_const_0 = &variable_global;
static int* pointer_global_static = &variable_global_static;
int** pointer_2d_global = &pointer_global;

outer_struct_t* outer_struct_ptr_global = &outer_struct_global;
const outer_struct_t* outer_struct_ptr_global_const_0 = &outer_struct_global;
static outer_struct_t* outer_struct_ptr_global_static = &outer_struct_global_static;

void test_remove_redundant_integer_casts()
{
    keep_variable_global_const(VARIABLE_GLOBAL_CONST);

    // Initialization of a variable
    int variable = -19;
    const int variable_const = 20;
    static int variable_static = -21;

    // Assignment to a variable
    variable_global = 22;
    variable_global_static = -23;
    variable = 24;
    variable_static = -25;
    assign_to_variable(variable);

    // Comparison with the value of a variable
    if (variable_global < 0x1A) {volatile int keep = 0;};
    if (variable_global_static <= -023) {volatile int keep = 0;};
    if (variable != 28) {volatile int keep = 0;}
    if (variable_const == -29) {volatile int keep = 0;}
    if (variable_static >= 30) {volatile int keep = 0;}
    compare_with_variable_value(variable, variable_const);

    // Assignment to a memory location pointed to by a pointer
    int* pointer = &variable;
    const int* pointer_const_0 = &variable_const;
    int* const pointer_const_1 = &variable;
    static int* pointer_static = &variable_static;
    int** pointer_2d = &pointer;
    *pointer_global = -0x1F;
    *pointer_global_static = 040;
    **pointer_2d_global = -33;
    *pointer = 34;
    *pointer_const_1 = -35;
    *pointer_static = 36;
    **pointer_2d = -37;
    assign_to_memory_location(pointer, pointer_const_1, pointer_2d);

    // Comparison with the value at a memory location pointed to by a pointer
    if (38 > *pointer_global) {volatile int keep = 0;}
    if (-39 < *pointer_global_const_0) {volatile int keep = 0;}
    if (40 <= *pointer_global_static) {volatile int keep = 0;}
    if (-41 != **pointer_2d_global) {volatile int keep = 0;}
    if (42 == *pointer) {volatile int keep = 0;}
    if (*pointer_const_0 >= -43) {volatile int keep = 0;}
    if (*pointer_const_1 > 44) {volatile int keep = 0;}
    if (*pointer_static < -45) {volatile int keep = 0;}
    if (**pointer_2d <= 46) {volatile int keep = 0;}
    compare_with_memory_location_value(pointer, pointer_const_0, pointer_const_1, pointer_2d);

    // Initialization of an array
    int array[4] = {-47, 48, -49, 50};
    const int array_const[4] = {-51, 52, -53, 54};
    static int array_static[4] = {-55, 56, -57, 58};
    int array_2d[2][2] = {{-59, 60}, {-61, 62}};

    // Assignment to an element of an array
    const int i = 0;
    array_global[0] = -63;
    array_global[i] = 64;
    array_global_static[0] = -65;
    array_global_static[i] = 66;
    array_2d_global[0][0] = -67;
    array_2d_global[i][i] = 68;
    array[0] = -69;
    array[i] = 70;
    array_static[0] = -71;
    array_static[i] = 72;
    array_2d[0][0] = -73;
    array_2d[i][i] = 74;

    // Comparison with the value of an element of an array
    if (array_global[0] != -75) {volatile int keep = 0;}
    if (76 == array_global_static[0]) {volatile int keep = 0;}
    if (-77 >= array_global[i]) {volatile int keep = 0;}
    if (78 > array_global_static[i]) {volatile int keep = 0;}
    if (-79 < array_2d_global[0][0]) {volatile int keep = 0;}
    if (80 <= array_2d_global[i][i]) {volatile int keep = 0;}
    if (array[0] != -81) {volatile int keep = 0;}
    if (array[i] == 82) {volatile int keep = 0;}
    if (array_const[0] >= -83) {volatile int keep = 0;}
    if (array_const[i] > 84) {volatile int keep = 0;}
    if (array_static[0] < -85) {volatile int keep = 0;}
    if (86 <= array_static[i]) {volatile int keep = 0;}
    if (-87 != array_2d[0][0]) {volatile int keep = 0;}
    if (88 == array_2d[i][i] ) {volatile int keep = 0;}

    // Initialization of a struct
    outer_struct_t outer_struct = { -89, { 90 } };
    const outer_struct_t outer_struct_const = { -91, { 92 } };
    static outer_struct_t outer_struct_static = { -93, { 94 } };

    // Assignment to a member of a struct
    outer_struct_global.x = -95;
    outer_struct_global.inner_struct.x = 96;
    outer_struct_global_static.x = -97;
    outer_struct_global_static.inner_struct.x = 98;
    outer_struct.x = -99;
    outer_struct.inner_struct.x = 100;
    outer_struct_static.x = -101;
    outer_struct_static.inner_struct.x = 102;
    assign_to_struct_member(outer_struct);

    // Comparison with the value of a member of a struct
    if (-103 >= outer_struct_global.x) {volatile int keep = 0;}
    if (104 > outer_struct_global.inner_struct.x) {volatile int keep = 0;}
    if (outer_struct_global_static.x < -105) {volatile int keep = 0;}
    if (outer_struct_global_static.inner_struct.x <= 106) {volatile int keep = 0;}
    if (outer_struct.x != -107) {volatile int keep = 0;}
    if (outer_struct.inner_struct.x == 108) {volatile int keep = 0;}
    if (outer_struct_const.x >= -109) {volatile int keep = 0;}
    if (110 > outer_struct_const.inner_struct.x) {volatile int keep = 0;}
    if (-111 < outer_struct_static.x) {volatile int keep = 0;}
    if (112 <= outer_struct_static.inner_struct.x) {volatile int keep = 0;}
    compare_with_struct_member_value
    (
        outer_struct,
        outer_struct_const
    );

    // Assignment to a member of a struct dereferenced via a pointer
    outer_struct_t* outer_struct_ptr = &outer_struct;
    const outer_struct_t* outer_struct_ptr_const_0 = &outer_struct_const;
    outer_struct_t* const outer_struct_ptr_const_1 = &outer_struct;
    static outer_struct_t* outer_struct_ptr_static = &outer_struct_static;
    outer_struct_ptr_global->x = -113;
    outer_struct_ptr_global->inner_struct.x = 114;
    outer_struct_ptr_global_static->x = -115;
    outer_struct_ptr_global_static->inner_struct.x = 116;
    outer_struct_ptr->x = -117;
    outer_struct_ptr->inner_struct.x = 118;
    outer_struct_ptr_const_1->x = -119;
    outer_struct_ptr_const_1->inner_struct.x = 120;
    outer_struct_ptr_static->x = -121;
    outer_struct_ptr_static->inner_struct.x = 122;

    assign_to_member_of_struct_dereferenced_via_pointer
    (
        outer_struct_ptr,
        outer_struct_ptr_const_1
    );

    // Comparison with the value of a member of a struct dereferenced via a pointer
    if (-123 != outer_struct_ptr_global->x) {volatile int keep = 0;}
    if (124 == outer_struct_ptr_global->inner_struct.x) {volatile int keep = 0;}
    if (outer_struct_ptr_global_const_0->x >= -125) {volatile int keep = 0;}
    if (outer_struct_ptr_global_const_0->inner_struct.x > 126) {volatile int keep = 0;}
    if (outer_struct_ptr_global_static->x < -127) {volatile int keep = 0;}
    if (outer_struct_ptr_global_static->inner_struct.x <= 128) {volatile int keep = 0;}
    if (outer_struct_ptr->x != -129) {volatile int keep = 0;}
    if (130 == outer_struct_ptr->inner_struct.x) {volatile int keep = 0;}
    if (-131 >= outer_struct_ptr_const_0->x) {volatile int keep = 0;}
    if (132 > outer_struct_ptr_const_0->inner_struct.x) {volatile int keep = 0;}
    if (-133 < outer_struct_ptr_const_1->x) {volatile int keep = 0;}
    if (134 <= outer_struct_ptr_const_1->inner_struct.x) {volatile int keep = 0;}
    if (outer_struct_ptr_static->x != -135) {volatile int keep = 0;}
    if (outer_struct_ptr_static->inner_struct.x == 136) {volatile int keep = 0;}
    compare_with_member_value_of_struct_dereferenced_via_pointer
    (
        outer_struct_ptr,
        outer_struct_ptr_const_0,
        outer_struct_ptr_const_1
    );

    // Comparison with the value returned by a function
    if (return_integer() >= -137) {volatile int keep = 0;}
    if (return_integer_extern() > 138) {volatile int keep = 0;}

    // Passing to a function
    receive_integer(-139);
    receive_integer_extern(140);

    // Return of a function
    return_integer();

    printf("End of test_remove_redundant_integer_casts().\n");
}

static void keep_variable_global_const(int variable_global_const) {}

static void assign_to_variable(int variable)
{
    variable = -141;
}

static void compare_with_variable_value(int variable, const int variable_const)
{
    if (variable < 142) {volatile int keep = 0;}
    if (-143 <= variable_const) {volatile int keep = 0;}
}

static void assign_to_memory_location(int* pointer, int* const pointer_const_1, int** pointer_2d)
{
    *pointer = 144;
    *pointer_const_1 = -145;
    **pointer_2d = 146;
}

static void compare_with_memory_location_value
(
    int* pointer,
    const int* pointer_const_0,
    int* const pointer_const_1,
    int** pointer_2d
)
{
    if (-147 != *pointer) {volatile int x = 0;}
    if (148 == *pointer_const_0) {volatile int x = 0;}
    if (-149 >= *pointer_const_1) {volatile int x = 0;}
    if (150 > **pointer_2d) {volatile int x = 0;}
}

static void assign_to_struct_member(outer_struct_t outer_struct)
{
    outer_struct.x = -151;
    outer_struct.inner_struct.x = 152;
}

static void compare_with_struct_member_value
(
    outer_struct_t outer_struct,
    const outer_struct_t outer_struct_const
)
{
    if (outer_struct.x < -153) {volatile int keep = 0;}
    if (outer_struct.inner_struct.x <= 154) {volatile int x = 0;}
    if (outer_struct_const.x != -155) {volatile int keep = 0;}
    if (outer_struct_const.inner_struct.x == 156) {volatile int x = 0;}
}

static void assign_to_member_of_struct_dereferenced_via_pointer
(
    outer_struct_t* outer_struct_ptr,
    outer_struct_t* const outer_struct_ptr_const_1
)
{
    outer_struct_ptr->x = -157;
    outer_struct_ptr->inner_struct.x = 158;
    outer_struct_ptr_const_1->x = -159;
    outer_struct_ptr_const_1->inner_struct.x = 160;
}

static void compare_with_member_value_of_struct_dereferenced_via_pointer
(
    outer_struct_t* outer_struct_ptr,
    const outer_struct_t* outer_struct_ptr_const_0,
    outer_struct_t* const outer_struct_ptr_const_1
)
{
    if (outer_struct_ptr->x >= -161) {volatile int keep = 0;}
    if (162 > outer_struct_ptr->inner_struct.x) {volatile int x = 0;}
    if (-163 < outer_struct_ptr_const_0->x) {volatile int keep = 0;}
    if (164 <= outer_struct_ptr_const_0->inner_struct.x) {volatile int x = 0;}
    if (-165 != outer_struct_ptr_const_1->x) {volatile int keep = 0;}
    if (166 == outer_struct_ptr_const_1->inner_struct.x) {volatile int x = 0;}
}

static void receive_integer(int a) {}

static int return_integer()
{
    return -167;
}
