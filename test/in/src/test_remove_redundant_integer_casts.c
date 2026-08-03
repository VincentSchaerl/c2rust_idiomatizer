#include <stdint.h>
#include <stdio.h>

#include "test_remove_redundant_integer_casts.h"
#include "nested_structs.h"

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

static int c;
static int* c_p = &c;
static int array2[4];
static struct OuterStruct outerStruct2;
static OuterStructTypedef outerStructTypedef2;
static struct OuterStruct* outerStruct2_p = &outerStruct2;
static OuterStructTypedef* outerStructTypedef2_p = &outerStructTypedef2;

void test_remove_redundant_integer_casts()
{
    // Initialization of a variable
    int a = 0;

    // Assignment to a variable
    int b;
    b = -1;
    b = 2;
    c = -3;
    assign_to_variable(a);

    // Comparison with the value of a variable
    if (b < 4) {volatile int keep = 0;}
    if (c <= -5) {volatile int keep = 0;}
    compare_with_variable_value(a);

    // Assignment to a memory location pointed to by a pointer
    int* a_p = &a;
    *a_p = 6;
    *c_p = -7;
    assign_to_memory_location(a_p);

    // Comparison with the value at a memory location pointed to by a pointer
    if (*a_p != 8) {volatile int keep = 0;}
    if (-9 == *c_p) {volatile int keep = 0;}
    compare_with_memory_location_value(a_p);

    // Initialization of an array
    int array0[4] = {012, -0xB, 12, -13};
    int array1[] = {14, -15, 16, -17};

    // Assignment to an element of an array
    a = 0;
    array0[0] = 18;
    array0[a] = -19;
    array1[0] = 20;
    array1[a] = -21;
    array2[0] = 22;
    array2[a] = -23;
    assign_to_array_element(array0);

    // Comparison with the value of an element of an array
    if (24 >= array0[0]) {volatile int keep = 0;}
    if (-031 > array0[a]) {volatile int keep = 0;}
    if (array1[0] < 0x1A) {volatile int keep = 0;}
    if (array1[a] <= -27) {volatile int keep = 0;}
    if (array2[0] != 28) {volatile int keep = 0;}
    if (-29 == array2[a]) {volatile int keep = 0;}
    compare_with_array_element_value(array0);

    // Initialization of a struct
    struct OuterStruct outerStruct0 = { 30, { -31 } };
    OuterStructTypedef outerStructTypedef0 = { 32, { -33 } };
    struct OuterStruct outerStruct1 = { .x = 34, .innerStruct = { .x = -35 } };
    OuterStructTypedef outerStructTypedef1 = { .x = 36, .innerStructTypedef = { .x = -37 } };

    // Assignment to a member of a struct
    outerStruct0.x = 38;
    outerStruct0.innerStruct.x = -39;
    outerStructTypedef0.x = 40;
    outerStructTypedef0.innerStructTypedef.x = -41;
    outerStruct1.x = 42;
    outerStruct1.innerStruct.x = -43;
    outerStructTypedef1.x = 44;
    outerStructTypedef1.innerStructTypedef.x = -45;
    outerStruct2.x = 46;
    outerStruct2.innerStruct.x = -47;
    outerStructTypedef2.x = 48;
    outerStructTypedef2.innerStructTypedef.x = -49;
    assign_to_struct_member(outerStruct0, outerStructTypedef0);

    // Comparison with the value of a member of a struct
    if (50 >= outerStruct0.x) {volatile int keep = 0;}
    if (-51 > outerStruct0.innerStruct.x) {volatile int x = 0;}
    if (outerStructTypedef0.x < 52) {volatile int keep = 0;}
    if (outerStructTypedef0.innerStructTypedef.x <= -53) {volatile int x = 0;}
    if (outerStruct1.x != 54) {volatile int keep = 0;}
    if (-55 == outerStruct1.innerStruct.x) {volatile int x = 0;}
    if (56 >= outerStructTypedef1.x) {volatile int keep = 0;}
    if (-57 > outerStructTypedef1.innerStructTypedef.x) {volatile int x = 0;}
    if (outerStruct2.x < 58) {volatile int keep = 0;}
    if (outerStruct2.innerStruct.x <= -59) {volatile int x = 0;}
    if (outerStructTypedef2.x != 60) {volatile int keep = 0;}
    if (-61 == outerStructTypedef2.innerStructTypedef.x) {volatile int x = 0;}
    compare_with_struct_member_value(outerStruct0, outerStructTypedef0);

    // Assignment to a member of a struct dereferenced via a pointer
    struct OuterStruct* outerStruct0_p = &outerStruct0;
    OuterStructTypedef* outerStructTypedef0_p = &outerStructTypedef0;
    struct OuterStruct* outerStruct1_p = &outerStruct1;
    OuterStructTypedef* outerStructTypedef1_p = &outerStructTypedef1;
    outerStruct0_p->x = 62;
    outerStruct0_p->innerStruct.x = -63;
    outerStructTypedef0_p->x = 64;
    outerStructTypedef0_p->innerStructTypedef.x = -65;
    outerStruct1_p->x = 66;
    outerStruct1_p->innerStruct.x = -67;
    outerStructTypedef1_p->x = 68;
    outerStructTypedef1_p->innerStructTypedef.x = -69;
    outerStruct2_p->x = 70;
    outerStruct2_p->innerStruct.x = -71;
    outerStructTypedef2_p->x = 72;
    outerStructTypedef2_p->innerStructTypedef.x = -73;
    assign_to_member_of_struct_dereferenced_via_pointer(&outerStruct0, &outerStructTypedef0);

    // Comparison with the value of a member of a struct dereferenced via a pointer
    if (74 >= outerStruct0_p->x) {volatile int keep = 0;}
    if (-75 > outerStruct0_p->innerStruct.x) {volatile int x = 0;}
    if (outerStructTypedef0_p->x < 76) {volatile int keep = 0;}
    if (outerStructTypedef0_p->innerStructTypedef.x <= -77) {volatile int x = 0;}
    if (outerStruct1_p->x != 78) {volatile int keep = 0;}
    if (-79 == outerStruct1_p->innerStruct.x) {volatile int x = 0;}
    if (80 >= outerStructTypedef1_p->x) {volatile int keep = 0;}
    if (-81 > outerStructTypedef1_p->innerStructTypedef.x) {volatile int x = 0;}
    if (outerStruct2_p->x < 82) {volatile int keep = 0;}
    if (outerStruct2_p->innerStruct.x <= -83) {volatile int x = 0;}
    if (outerStructTypedef2_p->x != 84) {volatile int keep = 0;}
    if (-85 == outerStructTypedef2_p->innerStructTypedef.x) {volatile int x = 0;}
    compare_with_member_value_of_struct_dereferenced_via_pointer(&outerStruct0, &outerStructTypedef0);

    // Passing to a function
    receive_integer(86);

    // Return of a function
    return_integer();

    printf("End of test_remove_redundant_integer_casts().\n");
}

static void assign_to_variable(int a)
{
    a = -87;
}

static void compare_with_variable_value(int a)
{
    if (88 >= a) {volatile int keep = 0;}
}

static void assign_to_memory_location(int* a_p)
{
    *a_p = -89;
}

static void compare_with_memory_location_value(int* a_p)
{
    if (90 > *a_p) {volatile int x = 0;}
}

static void assign_to_array_element(int array[])
{
    int a = 0;
    array[0] = -91;
    array[a] = 92;
}

static void compare_with_array_element_value(int array[])
{
    int a = 0;
    if (array[0] < -93) {volatile int keep = 0;}
    if (array[a] <= 94) {volatile int x = 0;}
}

static void assign_to_struct_member(struct OuterStruct outerStruct, OuterStructTypedef outerStructTypedef)
{
    outerStruct.x = -95;
    outerStruct.innerStruct.x = 96;
    outerStructTypedef.x = -97;
    outerStructTypedef.innerStructTypedef.x = 98;
}

static void compare_with_struct_member_value(struct OuterStruct outerStruct, OuterStructTypedef outerStructTypedef)
{
    if (outerStruct.x != -99) {volatile int keep = 0;}
    if (100 == outerStruct.innerStruct.x) {volatile int x = 0;}
    if (-101 >= outerStructTypedef.x) {volatile int keep = 0;}
    if (102 > outerStructTypedef.innerStructTypedef.x) {volatile int x = 0;}
}

static void assign_to_member_of_struct_dereferenced_via_pointer
(
    struct OuterStruct* outerStruct_p,
    OuterStructTypedef* outerStructTypedef_p
)
{
    outerStruct_p->x = -103;
    outerStruct_p->innerStruct.x = 104;
    outerStructTypedef_p->x = -105;
    outerStructTypedef_p->innerStructTypedef.x = 106;
}

static void compare_with_member_value_of_struct_dereferenced_via_pointer
(
    struct OuterStruct* outerStruct_p,
    OuterStructTypedef* outerStructTypedef_p
)
{
    if (outerStruct_p->x < -107) {volatile int keep = 0;}
    if (outerStruct_p->innerStruct.x <= 108) {volatile int x = 0;}
    if (outerStructTypedef_p->x != -109) {volatile int keep = 0;}
    if (110 == outerStructTypedef_p->innerStructTypedef.x) {volatile int x = 0;}
}

static void receive_integer(int a) {}

static int return_integer()
{
    return -111;
}
