#include <stdint.h>
#include <stdio.h>

#include "test_remove_redundant_integer_casts.h"
#include "nested_structs.h"

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

    // Assignment to a memory location pointed to by a pointer
    int* a_p = &a;
    *a_p = 4;
    *c_p = -5;
    assign_to_memory_location(a_p);

    // Initialization of an array
    int array0[4] = {6, -7, 8, -9};
    int array1[] = {012, -0xB, 12, -13};

    // Assignment to an element of an array
    a = 0;
    array0[0] = 14;
    array0[a] = -15;
    array1[0] = 16;
    array1[a] = -17;
    array2[0] = 18;
    array2[a] = -19;
    assign_to_array_element(array0);

    // Initialization of a struct
    struct OuterStruct outerStruct0 = { 20, { -21 } };
    OuterStructTypedef outerStructTypedef0 = { 22, { -23 } };
    struct OuterStruct outerStruct1 = { .x = 24, .innerStruct = { .x = -25 } };
    OuterStructTypedef outerStructTypedef1 = { .x = 26, .innerStructTypedef = { .x = -27 } };

    // Assignment to a member of a struct
    outerStruct0.x = 28;
    outerStruct0.innerStruct.x = -29;
    outerStructTypedef0.x = 30;
    outerStructTypedef0.innerStructTypedef.x = -31;
    outerStruct1.x = 32;
    outerStruct1.innerStruct.x = -33;
    outerStructTypedef1.x = 34;
    outerStructTypedef1.innerStructTypedef.x = -35;
    outerStruct2.x = 36;
    outerStruct2.innerStruct.x = -37;
    outerStructTypedef2.x = 38;
    outerStructTypedef2.innerStructTypedef.x = -39;
    assign_to_struct_member(outerStruct0, outerStructTypedef0);

    // Assignment to a member of a struct dereferenced via a pointer
    struct OuterStruct* outerStruct0_p = &outerStruct0;
    OuterStructTypedef* outerStructTypedef0_p = &outerStructTypedef0;
    struct OuterStruct* outerStruct1_p = &outerStruct1;
    OuterStructTypedef* outerStructTypedef1_p = &outerStructTypedef1;
    outerStruct0_p->x = 40;
    outerStruct0_p->innerStruct.x = -41;
    outerStructTypedef0_p->x = 42;
    outerStructTypedef0_p->innerStructTypedef.x = -43;
    outerStruct1_p->x = 44;
    outerStruct1_p->innerStruct.x = -45;
    outerStructTypedef1_p->x = 46;
    outerStructTypedef1_p->innerStructTypedef.x = -47;
    outerStruct2_p->x = 48;
    outerStruct2_p->innerStruct.x = -49;
    outerStructTypedef2_p->x = 50;
    outerStructTypedef2_p->innerStructTypedef.x = -51;
    assign_to_member_of_struct_dereferenced_via_a_pointer(&outerStruct0, &outerStructTypedef0);

    // Passing to a function
    receive_integer(52);

    // Return of a function
    return_integer();

    printf("End of test_remove_redundant_integer_casts().\n");
}

void assign_to_variable(int a)
{
    a = -53;
}

void assign_to_memory_location(int* a_p)
{
    *a_p = 54;
}

void assign_to_array_element(int array[])
{
    array[0] = -55;
}

void assign_to_struct_member(struct OuterStruct outerStruct, OuterStructTypedef outerStructTypedef)
{
    outerStruct.x = 56;
    outerStruct.innerStruct.x = -57;
    outerStructTypedef.x = 58;
    outerStructTypedef.innerStructTypedef.x = -59;
}

void assign_to_member_of_struct_dereferenced_via_a_pointer
(
    struct OuterStruct* outerStruct_p,
    OuterStructTypedef* outerStructTypedef_p
)
{
    outerStruct_p->x = 60;
    outerStruct_p->innerStruct.x = -61;
    outerStructTypedef_p->x = 62;
    outerStructTypedef_p->innerStructTypedef.x = -63;
}

void receive_integer(int a) {}

int return_integer()
{
    return 64;
}
