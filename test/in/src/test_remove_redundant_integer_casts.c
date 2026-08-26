#include <stdint.h>
#include <stdio.h>

#include "nested_structs.h"

static void keep_DEFINE(int define);
static void assign_to_variable(int variable);
static void compare_with_variable_value(int variable);
static void assign_to_memory_location(int* variable_ptr);
static void compare_with_memory_location_value(int* variable_ptr);
static void assign_to_array_element(int array[]);
static void compare_with_array_element_value(int array[]);
static void assign_to_struct_member(struct outer_struct_t outer_struct, outer_struct_typedef_t outer_struct_typedef);
static void compare_with_struct_member_value(struct outer_struct_t outer_struct, outer_struct_typedef_t outer_struct_typedef);
static void assign_to_member_of_struct_dereferenced_via_pointer
(
    struct outer_struct_t* outer_struct_ptr,
    outer_struct_typedef_t* outer_struct_typedef_ptr
);
static void compare_with_member_value_of_struct_dereferenced_via_pointer
(
    struct outer_struct_t* outer_struct_ptr,
    outer_struct_typedef_t* outer_struct_typedef_ptr
);
static void receive_integer(int a);
static int return_integer();

// Initialization of a variable
static int variable_global = 0;
#define DEFINE 1

// Initialization of an array
static int array_global[4] = {2, -3, 4, -5};

// Initialization of a struct
static struct outer_struct_t outer_struct_global = { 6, { -7 } };
static outer_struct_typedef_t outer_struct_typedef_global = { 8, { -9 } };

static int* variable_ptr_global = &variable_global;
static struct outer_struct_t* outer_struct_ptr_global = &outer_struct_global;
static outer_struct_typedef_t* outer_struct_typedef_ptr_global = &outer_struct_typedef_global;

void test_remove_redundant_integer_casts()
{
    keep_DEFINE(DEFINE);

    // Initialization of a variable
    int variable = 10;
    static int variable_static = -013;
    int variable_2 = 0xC == variable;

    // Assignment to a variable
    variable_global = -13;
    variable = 14;
    assign_to_variable(variable);

    // Comparison with the value of a variable
    if (variable_global < -0xF) {volatile int keep = 0;}
    if (variable <= 020) {volatile int keep = 0;}
    compare_with_variable_value(variable);

    // Assignment to a memory location pointed to by a pointer
    int* variable_ptr = &variable;
    *variable_ptr_global = -17;
    *variable_ptr = 18;
    assign_to_memory_location(variable_ptr);

    // Comparison with the value at a memory location pointed to by a pointer
    if (*variable_ptr_global != -19) {volatile int keep = 0;}
    if (20 == *variable_ptr) {volatile int keep = 0;}
    compare_with_memory_location_value(variable_ptr);

    // Initialization of an array
    int array[4] = {-21, 22, -23, 24};

    // Assignment to an element of an array
    int i = 0;
    array_global[0] = -25;
    array_global[i] = 26;
    array[0] = -27;
    array[i] = 28;
    assign_to_array_element(array);

    // Comparison with the value of an element of an array
    if (-29 >= array_global[0]) {volatile int keep = 0;}
    if (30 > array_global[i]) {volatile int keep = 0;}
    if (array[0] < -31) {volatile int keep = 0;}
    if (array[i] <= 32) {volatile int keep = 0;}
    compare_with_array_element_value(array);

    // Initialization of a struct
    struct outer_struct_t outer_struct = { -33, { 34 } };
    outer_struct_typedef_t outer_struct_typedef = { -35, { 36 } };

    // Assignment to a member of a struct
    outer_struct_global.x = -37;
    outer_struct_global.inner_struct.x = 38;
    outer_struct_typedef_global.x = -39;
    outer_struct_typedef_global.inner_struct_typedef.x = 40;
    outer_struct.x = -41;
    outer_struct.inner_struct.x = 42;
    outer_struct_typedef.x = -43;
    outer_struct_typedef.inner_struct_typedef.x = 44;
    assign_to_struct_member(outer_struct, outer_struct_typedef);

    // Comparison with the value of a member of a struct
    if (outer_struct_global.x != -45) {volatile int keep = 0;}
    if (46 == outer_struct_global.inner_struct.x) {volatile int x = 0;}
    if (-47 >= outer_struct_typedef_global.x) {volatile int keep = 0;}
    if (48 > outer_struct_typedef_global.inner_struct_typedef.x) {volatile int x = 0;}
    if (outer_struct.x < -49) {volatile int keep = 0;}
    if (outer_struct.inner_struct.x <= 50) {volatile int x = 0;}
    if (outer_struct_typedef.x != -51) {volatile int keep = 0;}
    if (52 == outer_struct_typedef.inner_struct_typedef.x) {volatile int x = 0;}
    compare_with_struct_member_value(outer_struct, outer_struct_typedef);

    // Assignment to a member of a struct dereferenced via a pointer
    struct outer_struct_t* outer_struct_ptr = &outer_struct;
    outer_struct_typedef_t* outer_struct_typedef_ptr = &outer_struct_typedef;
    outer_struct_ptr_global->x = -53;
    outer_struct_ptr_global->inner_struct.x = 54;
    outer_struct_typedef_ptr_global->x = -55;
    outer_struct_typedef_ptr_global->inner_struct_typedef.x = 56;
    outer_struct_ptr->x = -57;
    outer_struct_ptr->inner_struct.x = 58;
    outer_struct_typedef_ptr->x = -59;
    outer_struct_typedef_ptr->inner_struct_typedef.x = 60;
    assign_to_member_of_struct_dereferenced_via_pointer(outer_struct_ptr, outer_struct_typedef_ptr);

    // Comparison with the value of a member of a struct dereferenced via a pointer
    if (-61 >= outer_struct_ptr_global->x) {volatile int keep = 0;}
    if (62 > outer_struct_ptr_global->inner_struct.x) {volatile int x = 0;}
    if ( outer_struct_typedef_ptr_global->x < -63) {volatile int keep = 0;}
    if (outer_struct_typedef_ptr_global->inner_struct_typedef.x <= 64) {volatile int x = 0;}
    if (outer_struct_ptr->x != -65) {volatile int keep = 0;}
    if (66 == outer_struct_ptr->inner_struct.x) {volatile int x = 0;}
    if (-67 >= outer_struct_typedef_ptr->x) {volatile int keep = 0;}
    if (68 > outer_struct_typedef_ptr->inner_struct_typedef.x) {volatile int x = 0;}
    compare_with_member_value_of_struct_dereferenced_via_pointer(outer_struct_ptr, outer_struct_typedef_ptr);

    // Passing to a function
    receive_integer(-69);

    // Return of a function
    return_integer();

    printf("End of test_remove_redundant_integer_casts().\n");
}

static void keep_DEFINE(int define) {}

static void assign_to_variable(int variable)
{
    variable = 70;
}

static void compare_with_variable_value(int variable)
{
    if (variable < -71) {volatile int keep = 0;}
}

static void assign_to_memory_location(int* variable_ptr)
{
    *variable_ptr = 72;
}

static void compare_with_memory_location_value(int* variable_ptr)
{
    if (*variable_ptr <= -73) {volatile int x = 0;}
}

static void assign_to_array_element(int array[])
{
    int i = 0;
    array[0] = 74;
    array[i] = -75;
}

static void compare_with_array_element_value(int array[])
{
    int i = 0;
    if (array[0] != 76) {volatile int keep = 0;}
    if (-77 == array[i]) {volatile int x = 0;}
}

static void assign_to_struct_member(struct outer_struct_t outer_struct, outer_struct_typedef_t outer_struct_typedef)
{
    outer_struct.x = 78;
    outer_struct.inner_struct.x = -79;
    outer_struct_typedef.x = 80;
    outer_struct_typedef.inner_struct_typedef.x = -81;
}

static void compare_with_struct_member_value(struct outer_struct_t outer_struct, outer_struct_typedef_t outer_struct_typedef)
{
    if (82 >= outer_struct.x) {volatile int keep = 0;}
    if (-83 > outer_struct.inner_struct.x) {volatile int x = 0;}
    if (outer_struct_typedef.x < 84) {volatile int keep = 0;}
    if (outer_struct_typedef.inner_struct_typedef.x <= -85) {volatile int x = 0;}
}

static void assign_to_member_of_struct_dereferenced_via_pointer
(
    struct outer_struct_t* outer_struct_ptr,
    outer_struct_typedef_t* outer_struct_typedef_ptr
)
{
    outer_struct_ptr->x = 86;
    outer_struct_ptr->inner_struct.x = -87;
    outer_struct_typedef_ptr->x = 88;
    outer_struct_typedef_ptr->inner_struct_typedef.x = -89;
}

static void compare_with_member_value_of_struct_dereferenced_via_pointer
(
    struct outer_struct_t* outer_struct_ptr,
    outer_struct_typedef_t* outer_struct_typedef_ptr
)
{
    if (outer_struct_ptr->x != 90) {volatile int keep = 0;}
    if (-91 == outer_struct_ptr->inner_struct.x) {volatile int x = 0;}
    if (92 >= outer_struct_typedef_ptr->x) {volatile int keep = 0;}
    if (-93 > outer_struct_typedef_ptr->inner_struct_typedef.x) {volatile int x = 0;}
}

static void receive_integer(int a) {}

static int return_integer()
{
    return 94;
}
