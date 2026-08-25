#include <stdint.h>
#include <stdio.h>

#include "test_remove_redundant_integer_casts.h"
#include "nested_structs.h"

static void keep_DEFINE(int define);
static void assign_to_variable(int a);
static void compare_with_variable_value(int a);
static void assign_to_memory_location(int* a_p);
static void compare_with_memory_location_value(int* a_p);
static void assign_to_array_element(int array[]);
static void compare_with_array_element_value(int array[]);
static void assign_to_struct_member(struct OuterStruct outerStruct, OuterStructTypedef outerStructTypedef);
static void compare_with_struct_member_value(struct OuterStruct outerStruct, OuterStructTypedef outerStructTypedef);
static void assign_to_member_of_struct_dereferenced_via_pointer
(
    struct OuterStruct* outerStruct_p,
    OuterStructTypedef* outerStructTypedef_p
);
static void compare_with_member_value_of_struct_dereferenced_via_pointer
(
    struct OuterStruct* outerStruct_p,
    OuterStructTypedef* outerStructTypedef_p
);
static void receive_integer(int a);
static int return_integer();

// Initialization of a variable
static int a = 0;
#define DEFINE 1

// Initialization of an array
static int array0[4] = {2, -3, 4, -5};

// Initialization of a struct
static struct OuterStruct outerStruct0 = { 6, { -7 } };
static OuterStructTypedef outerStructTypedef0 = { 8, { -9 } };

static int* a_p = &a;
static struct OuterStruct* outerStruct0_p = &outerStruct0;
static OuterStructTypedef* outerStructTypedef0_p = &outerStructTypedef0;

void test_remove_redundant_integer_casts()
{
    keep_DEFINE(DEFINE);

    // Initialization of a variable
    int b = 10;
    static int c = -013;

    int d = 0xC == c;

    // Assignment to a variable
    a = -13;
    b = 14;
    assign_to_variable(a);

    // Comparison with the value of a variable
    if (a < -0xF) {volatile int keep = 0;}
    if (b <= 020) {volatile int keep = 0;}
    compare_with_variable_value(a);

    // Assignment to a memory location pointed to by a pointer
    int* b_p = &b;
    *a_p = -17;
    *b_p = 18;
    assign_to_memory_location(a_p);

    // Comparison with the value at a memory location pointed to by a pointer
    if (*a_p != -19) {volatile int keep = 0;}
    if (20 == *b_p) {volatile int keep = 0;}
    compare_with_memory_location_value(a_p);

    // Initialization of an array
    int array1[4] = {-21, 22, -23, 24};

    // Assignment to an element of an array
    a = 0;
    array0[0] = -25;
    array0[a] = 26;
    array1[0] = -27;
    array1[a] = 28;
    assign_to_array_element(array0);

    // Comparison with the value of an element of an array
    if (-29 >= array0[0]) {volatile int keep = 0;}
    if (30 > array0[a]) {volatile int keep = 0;}
    if (array1[0] < -31) {volatile int keep = 0;}
    if (array1[a] <= 32) {volatile int keep = 0;}
    compare_with_array_element_value(array0);

    // Initialization of a struct
    struct OuterStruct outerStruct1 = { -33, { 34 } };
    OuterStructTypedef outerStructTypedef1 = { -35, { 36 } };

    // Assignment to a member of a struct
    outerStruct0.x = -37;
    outerStruct0.innerStruct.x = 38;
    outerStructTypedef0.x = -39;
    outerStructTypedef0.innerStructTypedef.x = 40;
    outerStruct1.x = -41;
    outerStruct1.innerStruct.x = 42;
    outerStructTypedef1.x = -43;
    outerStructTypedef1.innerStructTypedef.x = 44;
    assign_to_struct_member(outerStruct0, outerStructTypedef0);

    // Comparison with the value of a member of a struct
    if (outerStruct0.x != -45) {volatile int keep = 0;}
    if (46 == outerStruct0.innerStruct.x) {volatile int x = 0;}
    if (-47 >= outerStructTypedef0.x) {volatile int keep = 0;}
    if (48 > outerStructTypedef0.innerStructTypedef.x) {volatile int x = 0;}
    if (outerStruct1.x < -49) {volatile int keep = 0;}
    if (outerStruct1.innerStruct.x <= 50) {volatile int x = 0;}
    if (outerStructTypedef1.x != -51) {volatile int keep = 0;}
    if (52 == outerStructTypedef1.innerStructTypedef.x) {volatile int x = 0;}
    compare_with_struct_member_value(outerStruct0, outerStructTypedef0);

    // Assignment to a member of a struct dereferenced via a pointer
    struct OuterStruct* outerStruct1_p = &outerStruct1;
    OuterStructTypedef* outerStructTypedef1_p = &outerStructTypedef1;
    outerStruct0_p->x = -53;
    outerStruct0_p->innerStruct.x = 54;
    outerStructTypedef0_p->x = -55;
    outerStructTypedef0_p->innerStructTypedef.x = 56;
    outerStruct1_p->x = -57;
    outerStruct1_p->innerStruct.x = 58;
    outerStructTypedef1_p->x = -59;
    outerStructTypedef1_p->innerStructTypedef.x = 60;
    assign_to_member_of_struct_dereferenced_via_pointer(&outerStruct0, &outerStructTypedef0);

    // Comparison with the value of a member of a struct dereferenced via a pointer
    if (-61 >= outerStruct0_p->x) {volatile int keep = 0;}
    if (62 > outerStruct0_p->innerStruct.x) {volatile int x = 0;}
    if ( outerStructTypedef0_p->x < -63) {volatile int keep = 0;}
    if (outerStructTypedef0_p->innerStructTypedef.x <= 64) {volatile int x = 0;}
    if (outerStruct1_p->x != -65) {volatile int keep = 0;}
    if (66 == outerStruct1_p->innerStruct.x) {volatile int x = 0;}
    if (-67 >= outerStructTypedef1_p->x) {volatile int keep = 0;}
    if (68 > outerStructTypedef1_p->innerStructTypedef.x) {volatile int x = 0;}
    compare_with_member_value_of_struct_dereferenced_via_pointer(&outerStruct0, &outerStructTypedef0);

    // Passing to a function
    receive_integer(-69);

    // Return of a function
    return_integer();

    printf("End of test_remove_redundant_integer_casts().\n");
}

static void keep_DEFINE(int define) {}

static void assign_to_variable(int a)
{
    a = 70;
}

static void compare_with_variable_value(int a)
{
    if (a < -71) {volatile int keep = 0;}
}

static void assign_to_memory_location(int* a_p)
{
    *a_p = 72;
}

static void compare_with_memory_location_value(int* a_p)
{
    if (*a_p <= -73) {volatile int x = 0;}
}

static void assign_to_array_element(int array[])
{
    int a = 0;
    array[0] = 74;
    array[a] = -75;
}

static void compare_with_array_element_value(int array[])
{
    int a = 0;
    if (array[0] != 76) {volatile int keep = 0;}
    if (-77 == array[a]) {volatile int x = 0;}
}

static void assign_to_struct_member(struct OuterStruct outerStruct, OuterStructTypedef outerStructTypedef)
{
    outerStruct.x = 78;
    outerStruct.innerStruct.x = -79;
    outerStructTypedef.x = 80;
    outerStructTypedef.innerStructTypedef.x = -81;
}

static void compare_with_struct_member_value(struct OuterStruct outerStruct, OuterStructTypedef outerStructTypedef)
{
    if (82 >= outerStruct.x) {volatile int keep = 0;}
    if (-83 > outerStruct.innerStruct.x) {volatile int x = 0;}
    if (outerStructTypedef.x < 84) {volatile int keep = 0;}
    if (outerStructTypedef.innerStructTypedef.x <= -85) {volatile int x = 0;}
}

static void assign_to_member_of_struct_dereferenced_via_pointer
(
    struct OuterStruct* outerStruct_p,
    OuterStructTypedef* outerStructTypedef_p
)
{
    outerStruct_p->x = 86;
    outerStruct_p->innerStruct.x = -87;
    outerStructTypedef_p->x = 88;
    outerStructTypedef_p->innerStructTypedef.x = -89;
}

static void compare_with_member_value_of_struct_dereferenced_via_pointer
(
    struct OuterStruct* outerStruct_p,
    OuterStructTypedef* outerStructTypedef_p
)
{
    if (outerStruct_p->x != 90) {volatile int keep = 0;}
    if (-91 == outerStruct_p->innerStruct.x) {volatile int x = 0;}
    if (92 >= outerStructTypedef_p->x) {volatile int keep = 0;}
    if (-93 > outerStructTypedef_p->innerStructTypedef.x) {volatile int x = 0;}
}

static void receive_integer(int a) {}

static int return_integer()
{
    return 94;
}
