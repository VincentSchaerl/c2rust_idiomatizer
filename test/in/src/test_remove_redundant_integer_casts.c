#include <stdint.h>
#include <stdio.h>

#include "nested_struct.h"

static void keep_variable_global_const(int variable_global_const);
static void assign_to_variable(int variable);
static void compare_with_variable_value(int variable, const int variable_const);
static void assign_to_memory_location(int* pointer, int* const pointer_const_1);
static void compare_with_memory_location_value
(
    int* pointer,
    const int* pointer_const_0,
    int* const pointer_const_1
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

// Initialization of a struct
outer_struct_t outer_struct_global = { -11, { 12 } };
static outer_struct_t outer_struct_global_static = { -13, { 14 } };

int* pointer_global = &variable_global;
const int* pointer_global_const_0 = &variable_global;
static int* pointer_global_static = &variable_global_static;

outer_struct_t* outer_struct_ptr_global = &outer_struct_global;
const outer_struct_t* outer_struct_ptr_global_const_0 = &outer_struct_global;
static outer_struct_t* outer_struct_ptr_global_static = &outer_struct_global_static;

void test_remove_redundant_integer_casts()
{
    keep_variable_global_const(VARIABLE_GLOBAL_CONST);

    // Initialization of a variable
    int variable = -0xF;
    const int variable_const = 020;
    static int variable_static = -17;

    // Assignment to a variable
    variable_global = 18;
    variable_global_static = -19;
    variable = 20;
    variable_static = -21;
    assign_to_variable(variable);

    // Comparison with the value of a variable
    if (variable_global < 22) {volatile int keep = 0;};
    if (variable_global_static <= -23) {volatile int keep = 0;};
    if (variable != 24) {volatile int keep = 0;}
    if (variable_const == -031) {volatile int keep = 0;}
    if (variable_static >= 0x1A) {volatile int keep = 0;}
    compare_with_variable_value(variable, variable_const);

    // Assignment to a memory location pointed to by a pointer
    int* pointer = &variable;
    const int* pointer_const_0 = &variable_const;
    int* const pointer_const_1 = &variable;
    static int* pointer_static = &variable_static;
    *pointer_global = -27;
    *pointer_global_static = 28;
    *pointer = -29;
    *pointer_const_1 = 30;
    *pointer_static = -31;
    assign_to_memory_location(pointer, pointer_const_1);

    // Comparison with the value at a memory location pointed to by a pointer
    if (32 > *pointer_global) {volatile int keep = 0;}
    if (-33 < *pointer_global_const_0) {volatile int keep = 0;}
    if (34 <= *pointer_global_static) {volatile int keep = 0;}
    if (-35 != *pointer) {volatile int keep = 0;}
    if (36 == *pointer_const_0) {volatile int keep = 0;}
    if (*pointer_const_1 >= -37) {volatile int keep = 0;}
    if (*pointer_static > 38) {volatile int keep = 0;}
    compare_with_memory_location_value(pointer, pointer_const_0, pointer_const_1);

    // Initialization of an array
    int array[4] = {-39, 40, -41, 42};
    const int array_const[4] = {-43, 44, -45, 46};
    static int array_static[4] = {-47, 48, -49, 50};

    // Assignment to an element of an array
    const int i = 0;
    array_global[0] = -51;
    array_global[i] = 52;
    array_global_static[0] = -53;
    array_global_static[i] = 54;
    array[0] = -55;
    array[i] = 56;
    array_static[0] = -57;
    array_static[i] = 58;

    // Comparison with the value of an element of an array
    if (array_global[0] < -59) {volatile int keep = 0;}
    if (array_global[i] <= 60) {volatile int keep = 0;}
    if (array_global_static[0] != -61) {volatile int keep = 0;}
    if (62 == array_global_static[i]) {volatile int keep = 0;}
    if (-63 >= array[0]) {volatile int keep = 0;}
    if (64 > array[i]) {volatile int keep = 0;}
    if (-65 < array_const[0]) {volatile int keep = 0;}
    if (66 <= array_const[i]) {volatile int keep = 0;}
    if (array_static[0] != -67) {volatile int keep = 0;}
    if (array_static[i] == 68) {volatile int keep = 0;}

    // Initialization of a struct
    outer_struct_t outer_struct = { -69, { 70 } };
    const outer_struct_t outer_struct_const = { -71, { 72 } };
    static outer_struct_t outer_struct_static = { -73, { 74 } };

    // Assignment to a member of a struct
    outer_struct_global.x = -75;
    outer_struct_global.inner_struct.x = 76;
    outer_struct_global_static.x = -77;
    outer_struct_global_static.inner_struct.x = 78;
    outer_struct.x = -79;
    outer_struct.inner_struct.x = 80;
    outer_struct_static.x = -81;
    outer_struct_static.inner_struct.x = 82;
    assign_to_struct_member(outer_struct);

    // Comparison with the value of a member of a struct
    if (outer_struct_global.x >= -83) {volatile int keep = 0;}
    if (outer_struct_global.inner_struct.x > 84) {volatile int keep = 0;}
    if (outer_struct_global_static.x < -85) {volatile int keep = 0;}
    if (86 <= outer_struct_global_static.inner_struct.x) {volatile int keep = 0;}
    if (-87 != outer_struct.x) {volatile int keep = 0;}
    if (88 == outer_struct.inner_struct.x) {volatile int keep = 0;}
    if (-89 >= outer_struct_const.x) {volatile int keep = 0;}
    if (90 < outer_struct_const.inner_struct.x) {volatile int keep = 0;}
    if (outer_struct_static.x <= -91) {volatile int keep = 0;}
    if (outer_struct_static.inner_struct.x != 92) {volatile int keep = 0;}
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
    outer_struct_ptr_global->x = -93;
    outer_struct_ptr_global->inner_struct.x = 94;
    outer_struct_ptr_global_static->x = -95;
    outer_struct_ptr_global_static->inner_struct.x = 96;
    outer_struct_ptr->x = -97;
    outer_struct_ptr->inner_struct.x = 98;
    outer_struct_ptr_const_1->x = -99;
    outer_struct_ptr_const_1->inner_struct.x = 100;
    outer_struct_ptr_static->x = -101;
    outer_struct_ptr_static->inner_struct.x = 102;

    assign_to_member_of_struct_dereferenced_via_pointer
    (
        outer_struct_ptr,
        outer_struct_ptr_const_1
    );

    // Comparison with the value of a member of a struct dereferenced via a pointer
    if (outer_struct_ptr_global->x == -103) {volatile int keep = 0;}
    if (outer_struct_ptr_global->inner_struct.x >= 104) {volatile int keep = 0;}
    if (outer_struct_ptr_global_const_0->x > -105) {volatile int keep = 0;}
    if (106 < outer_struct_ptr_global_const_0->inner_struct.x) {volatile int keep = 0;}
    if (-107 <= outer_struct_ptr_global_static->x) {volatile int keep = 0;}
    if (108 != outer_struct_ptr_global_static->inner_struct.x) {volatile int keep = 0;}
    if (-109 == outer_struct_ptr->x) {volatile int keep = 0;}
    if (110 >= outer_struct_ptr->inner_struct.x) {volatile int keep = 0;}
    if (outer_struct_ptr_const_0->x > -111) {volatile int keep = 0;}
    if (outer_struct_ptr_const_0->inner_struct.x < 112) {volatile int keep = 0;}
    if (outer_struct_ptr_const_1->x <= -113) {volatile int keep = 0;}
    if (outer_struct_ptr_const_1->inner_struct.x != 114) {volatile int keep = 0;}
    if (outer_struct_ptr_static->x == -115) {volatile int keep = 0;}
    if (116 >= outer_struct_ptr_static->inner_struct.x) {volatile int keep = 0;}
    compare_with_member_value_of_struct_dereferenced_via_pointer
    (
        outer_struct_ptr,
        outer_struct_ptr_const_0,
        outer_struct_ptr_const_1
    );

    // Passing to a function
    receive_integer(-117);

    // Return of a function
    return_integer();

    printf("End of test_remove_redundant_integer_casts().\n");
}

static void keep_variable_global_const(int variable_global_const) {}

static void assign_to_variable(int variable)
{
    variable = 118;
}

static void compare_with_variable_value(int variable, const int variable_const)
{
    if (-119 > variable) {volatile int keep = 0;}
    if (120 < variable_const) {volatile int keep = 0;}
}

static void assign_to_memory_location(int* pointer, int* const pointer_const_1)
{
    *pointer = -121;
    *pointer_const_1 = 122;
}

static void compare_with_memory_location_value
(
    int* pointer,
    const int* pointer_const_0,
    int* const pointer_const_1
)
{
    if (-123 <= *pointer) {volatile int x = 0;}
    if (124 != *pointer_const_0) {volatile int x = 0;}
    if (*pointer_const_1 == -125) {volatile int x = 0;}
}

static void assign_to_struct_member(outer_struct_t outer_struct)
{
    outer_struct.x = 126;
    outer_struct.inner_struct.x = -127;
}

static void compare_with_struct_member_value
(
    outer_struct_t outer_struct,
    const outer_struct_t outer_struct_const
)
{
    if (outer_struct.x >= 128) {volatile int keep = 0;}
    if (outer_struct.inner_struct.x > -129) {volatile int x = 0;}
    if (outer_struct_const.x < 130) {volatile int keep = 0;}
    if (outer_struct_const.inner_struct.x <= -131) {volatile int x = 0;}
}

static void assign_to_member_of_struct_dereferenced_via_pointer
(
    outer_struct_t* outer_struct_ptr,
    outer_struct_t* const outer_struct_ptr_const_1
)
{
    outer_struct_ptr->x = 132;
    outer_struct_ptr->inner_struct.x = -133;
    outer_struct_ptr_const_1->x = 134;
    outer_struct_ptr_const_1->inner_struct.x = -135;
}

static void compare_with_member_value_of_struct_dereferenced_via_pointer
(
    outer_struct_t* outer_struct_ptr,
    const outer_struct_t* outer_struct_ptr_const_0,
    outer_struct_t* const outer_struct_ptr_const_1
)
{
    if (136 != outer_struct_ptr->x) {volatile int keep = 0;}
    if (-137 == outer_struct_ptr->inner_struct.x) {volatile int x = 0;}
    if (138 >= outer_struct_ptr_const_0->x) {volatile int keep = 0;}
    if (-139 > outer_struct_ptr_const_0->inner_struct.x) {volatile int x = 0;}
    if (140 < outer_struct_ptr_const_1->x) {volatile int keep = 0;}
    if (outer_struct_ptr_const_1->inner_struct.x <= -141) {volatile int x = 0;}
}

static void receive_integer(int a) {}

static int return_integer()
{
    return 142;
}
