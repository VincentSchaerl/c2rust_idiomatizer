#include <stdint.h>
#include <stdio.h>

#include "test_1_remove_redundant_integer_casts.h"
#include "test_2_remove_redundant_integer_casts_support.h"
#include "nested_struct.h"
#include "typedefs.h"

static void keep_variable_global_const(int variable_global_const);
static void assign_to_variable(int variable, typedef_int variable_typedef);
static void compare_with_variable_value(int variable, const int variable_const, typedef_int variable_typedef);
static void assign_to_memory_location
(
    int* pointer,
    int* const pointer_const_1,
    typedef_int* pointer_typedef,
    int** pointer_2d
);
static void compare_with_memory_location_value
(
    int* pointer,
    const int* pointer_const_0,
    int* const pointer_const_1,
    typedef_int* pointer_typedef,
    int** pointer_2d
);
static void assign_to_struct_member(outer_struct_t outer_struct, outer_struct_typedef_t outer_struct_typedef);
static void compare_with_struct_member_value
(
    outer_struct_t outer_struct,
    const outer_struct_t outer_struct_const,
    outer_struct_typedef_t outer_struct_typedef
);
static void assign_to_member_of_struct_dereferenced_via_pointer
(
    outer_struct_t* outer_struct_ptr,
    outer_struct_t* const outer_struct_ptr_const_1,
    outer_struct_typedef_t* outer_struct_typedef
);
static void compare_with_member_value_of_struct_dereferenced_via_pointer
(
    outer_struct_t* outer_struct_ptr,
    const outer_struct_t* outer_struct_ptr_const_0,
    outer_struct_t* const outer_struct_ptr_const_1,
    outer_struct_typedef_t* outer_struct_typedef
);
static void receive_integer(int a);
static void receive_integer_typedef(typedef_int a);
static int return_integer();
static typedef_int return_integer_typedef();

// Initialization of a variable
int variable_global = 0;
#define VARIABLE_GLOBAL_CONST (-1)
static int variable_global_static = 2;

// Initialization of an array
int array_global_0[4] = {-3, 4, -5, 6};
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

void test_1_remove_redundant_integer_casts()
{
    keep_variable_global_const(VARIABLE_GLOBAL_CONST);

    // negative cases
    char false_0 = 111;
    typedef_char false_1 = -111;
    int8_t false_2 = 666;
    uint32_t false_3 = -666;

    // Initialization of a variable
    int variable = -19;
    const int variable_const = 20;
    static int variable_static = -21;
    typedef_signed_char variable_typedef_signed_char = 22;
    typedef_unsigned_char variable_typedef_unsigned_char = 23;
    typedef_short variable_typedef_signed_short = 24;
    typedef_unsigned_short variable_typedef_unsigned_short = 25;
    typedef_int variable_typedef_int = 26;
    typedef_unsigned_int variable_typedef_unsigned_int = 27;
    typedef_long variable_typedef_long = 28;
    typedef_unsigned_long variable_typedef_unsigned_long = 29;
    typedef_long_long variable_typedef_long_long = 30;
    typedef_unsigned_long_long variable_typedef_unsigned_long_long = 31;
    typedef_int8_t variable_typedef_int8_t = 32;
    typedef_uint8_t variable_typedef_uint8_t = 33;
    typedef_int16_t variable_typedef_int16_t = 34;
    typedef_uint16_t variable_typedef_uint16_t = 35;
    typedef_int32_t variable_typedef_int32_t = 36;
    typedef_uint32_t variable_typedef_uint32_t = 37;
    typedef_int64_t variable_typedef_int64_t = 38;
    typedef_uint64_t variable_typedef_uint64_t = 39;
    typedef_intptr_t variable_typedef_intptr_t = 40;
    typedef_uintptr_t variable_typedef_uintptr_t = 41;
    typedef_size_t variable_typedef_size_t = 42;

    // Assignment to a variable
    variable_global = -43;
    variable_global_static = 44;
    variable = -45;
    variable_static = 46;
    variable_typedef_int = -47;
    assign_to_variable(variable, variable_typedef_int);

    // Comparison with the value of a variable
    if (variable_global < 0x30) {volatile int keep = 0;};
    if (variable_global_static <= -061) {volatile int keep = 0;};
    if (variable != 50) {volatile int keep = 0;}
    if (variable_const == -51) {volatile int keep = 0;}
    if (variable_static >= 52) {volatile int keep = 0;}
    if (-53 > variable_typedef_int) {volatile int keep = 0;}
    compare_with_variable_value(variable, variable_const, variable_typedef_int);

    // Assignment to a memory location pointed to by a pointer
    int* pointer = &variable;
    const int* pointer_const_0 = &variable_const;
    int* const pointer_const_1 = &variable;
    static int* pointer_static = &variable_static;
    typedef_int* pointer_typedef = &variable_typedef_int;
    int** pointer_2d = &pointer;
    *pointer_global = 0x36;
    *pointer_global_static = -067;
    **pointer_2d_global = 56;
    *pointer = -57;
    *pointer_const_1 = 58;
    *pointer_static = -59;
    *pointer_typedef = 60;
    **pointer_2d = -61;
    assign_to_memory_location(pointer, pointer_const_1, pointer_typedef, pointer_2d);

    // Comparison with the value at a memory location pointed to by a pointer
    if (62 < *pointer_global) {volatile int keep = 0;}
    if (-63 <= *pointer_global_const_0) {volatile int keep = 0;}
    if (64 != *pointer_global_static) {volatile int keep = 0;}
    if (-65 == **pointer_2d_global) {volatile int keep = 0;}
    if (*pointer >= 66) {volatile int keep = 0;}
    if (*pointer_const_0 > -67) {volatile int keep = 0;}
    if (*pointer_const_1 < 68) {volatile int keep = 0;}
    if (*pointer_static <= -69) {volatile int keep = 0;}
    if (*pointer_typedef != 70) {volatile int keep = 0;}
    if (-71 == **pointer_2d) {volatile int keep = 0;}
    compare_with_memory_location_value(pointer, pointer_const_0, pointer_const_1, pointer_typedef, pointer_2d);

    // Initialization of an array
    int array[4] = {72, -73, 74, -75};
    const int array_const[4] = {76, -77, 78, -79};
    static int array_static[4] = {80, -81, 82, -83};
    typedef_int array_typedef[4] = {84, -85, 86, -87};
    int array_2d[2][2] = {{88, -89}, {90, -91}};

    // Assignment to an element of an array
    const int i = 0;
    array_global_0[0] = 92;
    array_global_0[i] = -93;
    array_global_static[0] = 94;
    array_global_static[i] = -95;
    array_2d_global[0][0] = 96;
    array_2d_global[i][i] = -97;
    array_2d_global_extern[0][0] = 98;
    array[0] = -99;
    array[i] = 100;
    array_static[0] = -101;
    array_static[i] = 102;
    array_typedef[0] = -103;
    array_typedef[i] = 104;
    array_2d[0][0] = -105;
    array_2d[i][i] = 106;

    // Comparison with the value of an element of an array
    if (-107 >= array_global_0[0]) {volatile int keep = 0;}
    if (108 > array_global_static[0]) {volatile int keep = 0;}
    if (-109 < array_global_0[i]) {volatile int keep = 0;}
    if (110 <= array_global_static[i]) {volatile int keep = 0;}
    if (array_2d_global[0][0] != -111) {volatile int keep = 0;}
    if (array_2d_global[i][i] == 112) {volatile int keep = 0;}
    if (array[0] >= -113) {volatile int keep = 0;}
    if (array[i] > 114) {volatile int keep = 0;}
    if (array_const[0] < -115) {volatile int keep = 0;}
    if (116 <= array_const[i]) {volatile int keep = 0;}
    if (-117 != array_static[0]) {volatile int keep = 0;}
    if (118 == array_static[i]) {volatile int keep = 0;}
    if (-119 >= array_typedef[0]) {volatile int keep = 0;}
    if (120 > array_typedef[i]) {volatile int keep = 0;}
    if (array_2d[0][0] < -121) {volatile int keep = 0;}
    if (array_2d[i][i] <= 122) {volatile int keep = 0;}

    // Initialization of a struct
    outer_struct_t outer_struct = { -123, { 124 } };
    const outer_struct_t outer_struct_const = { -125, { 126 } };
    static outer_struct_t outer_struct_static = { -127, { 128 } };
    outer_struct_typedef_t outer_struct_typedef = { -129, { 130 } };

    // Assignment to a member of a struct
    outer_struct_global.x = -131;
    outer_struct_global.inner_struct.x = 132;
    outer_struct_global_static.x = -133;
    outer_struct_global_static.inner_struct.x = 134;
    outer_struct.x = -135;
    outer_struct.inner_struct.x = 136;
    outer_struct_static.x = -137;
    outer_struct_static.inner_struct.x = 138;
    outer_struct_typedef.x = -139;
    outer_struct_typedef.inner_struct.x = 140;
    assign_to_struct_member(outer_struct, outer_struct_typedef);

    // Comparison with the value of a member of a struct
    if (outer_struct_global.x != -141) {volatile int keep = 0;}
    if (outer_struct_global.inner_struct.x == 142) {volatile int keep = 0;}
    if (outer_struct_global_static.x >= -143) {volatile int keep = 0;}
    if (144 > outer_struct_global_static.inner_struct.x) {volatile int keep = 0;}
    if (-145 < outer_struct.x) {volatile int keep = 0;}
    if (146 <= outer_struct.inner_struct.x) {volatile int keep = 0;}
    if (-147 != outer_struct_const.x) {volatile int keep = 0;}
    if (148 == outer_struct_const.inner_struct.x) {volatile int keep = 0;}
    if (outer_struct_static.x >= -149) {volatile int keep = 0;}
    if (outer_struct_static.inner_struct.x > 150) {volatile int keep = 0;}
    if (outer_struct_typedef.x < -151) {volatile int keep = 0;}
    if (outer_struct_typedef.inner_struct.x <= 152) {volatile int keep = 0;}
    compare_with_struct_member_value
    (
        outer_struct,
        outer_struct_const,
        outer_struct_typedef
    );

    // Assignment to a member of a struct dereferenced via a pointer
    outer_struct_t* outer_struct_ptr = &outer_struct;
    const outer_struct_t* outer_struct_ptr_const_0 = &outer_struct_const;
    outer_struct_t* const outer_struct_ptr_const_1 = &outer_struct;
    static outer_struct_t* outer_struct_ptr_static = &outer_struct_static;
    outer_struct_typedef_t* outer_struct_ptr_typedef = &outer_struct_typedef;
    outer_struct_ptr_global->x = -153;
    outer_struct_ptr_global->inner_struct.x = 154;
    outer_struct_ptr_global_static->x = -155;
    outer_struct_ptr_global_static->inner_struct.x = 156;
    outer_struct_ptr->x = -157;
    outer_struct_ptr->inner_struct.x = 158;
    outer_struct_ptr_const_1->x = -159;
    outer_struct_ptr_const_1->inner_struct.x = 160;
    outer_struct_ptr_static->x = -161;
    outer_struct_ptr_static->inner_struct.x = 162;
    outer_struct_ptr_typedef->x = -163;
    outer_struct_ptr_typedef->inner_struct.x = 164;

    assign_to_member_of_struct_dereferenced_via_pointer
    (
        outer_struct_ptr,
        outer_struct_ptr_const_1,
        outer_struct_ptr_typedef
    );

    // Comparison with the value of a member of a struct dereferenced via a pointer
    if (outer_struct_ptr_global->x != -165) {volatile int keep = 0;}
    if (166 == outer_struct_ptr_global->inner_struct.x) {volatile int keep = 0;}
    if (-167 >= outer_struct_ptr_global_const_0->x) {volatile int keep = 0;}
    if (168 > outer_struct_ptr_global_const_0->inner_struct.x) {volatile int keep = 0;}
    if (-169 < outer_struct_ptr_global_static->x) {volatile int keep = 0;}
    if (170 <= outer_struct_ptr_global_static->inner_struct.x) {volatile int keep = 0;}
    if (outer_struct_ptr->x != -171) {volatile int keep = 0;}
    if (outer_struct_ptr->inner_struct.x == 172) {volatile int keep = 0;}
    if (outer_struct_ptr_const_0->x >= -173) {volatile int keep = 0;}
    if (outer_struct_ptr_const_0->inner_struct.x > 174) {volatile int keep = 0;}
    if (outer_struct_ptr_const_1->x < -175) {volatile int keep = 0;}
    if (176 <= outer_struct_ptr_const_1->inner_struct.x) {volatile int keep = 0;}
    if (-177 != outer_struct_ptr_static->x) {volatile int keep = 0;}
    if (178 == outer_struct_ptr_static->inner_struct.x) {volatile int keep = 0;}
    if (-179 >= outer_struct_ptr_typedef->x) {volatile int keep = 0;}
    if (180 > outer_struct_ptr_typedef->inner_struct.x) {volatile int keep = 0;}
    compare_with_member_value_of_struct_dereferenced_via_pointer
    (
        outer_struct_ptr,
        outer_struct_ptr_const_0,
        outer_struct_ptr_const_1,
        outer_struct_ptr_typedef
    );

    // Comparison with the value returned by a function
    if (return_integer() < -181) {volatile int keep = 0;}
    if (return_integer_extern() <= 182) {volatile int keep = 0;}
    if (return_integer_typedef() != -183) {volatile int keep = 0;}

    // Passing to a function
    receive_integer(184);
    receive_integer_extern(-185);
    receive_integer_typedef(186);

    // Return of a function
    return_integer();
    return_integer_typedef();

    printf("End of test_1_remove_redundant_integer_casts().\n");
}

static void keep_variable_global_const(int variable_global_const) {}

static void assign_to_variable(int variable, typedef_int variable_typedef)
{
    variable = -187;
    variable_typedef = 188;
}

static void compare_with_variable_value(int variable, const int variable_const, typedef_int variable_typedef)
{
    if (variable == -189) {volatile int keep = 0;}
    if (variable_const >= 190) {volatile int keep = 0;}
    if (-191 > variable_typedef) {volatile int keep = 0;}
}

static void assign_to_memory_location
(
    int* pointer,
    int* const pointer_const_1,
    typedef_int* pointer_typedef,
    int** pointer_2d
)
{
    *pointer = 192;
    *pointer_const_1 = -193;
    *pointer_typedef = 194;
    **pointer_2d = -195;
}

static void compare_with_memory_location_value
(
    int* pointer,
    const int* pointer_const_0,
    int* const pointer_const_1,
    typedef_int* pointer_typedef,
    int** pointer_2d
)
{
    if (196 < *pointer) {volatile int x = 0;}
    if (-197 <= *pointer_const_0) {volatile int x = 0;}
    if (198 != *pointer_const_1) {volatile int x = 0;}
    if (-199 == *pointer_typedef) {volatile int x = 0;}
    if (**pointer_2d >= 200) {volatile int x = 0;}
}

static void assign_to_struct_member(outer_struct_t outer_struct, outer_struct_typedef_t outer_struct_typedef)
{
    outer_struct.x = -201;
    outer_struct.inner_struct.x = 202;
    outer_struct_typedef.x = -203;
    outer_struct_typedef.inner_struct.x = 204;
}

static void compare_with_struct_member_value
(
    outer_struct_t outer_struct,
    const outer_struct_t outer_struct_const,
    outer_struct_typedef_t outer_struct_typedef
)
{
    if (outer_struct.x > -205) {volatile int keep = 0;}
    if (outer_struct.inner_struct.x < 206) {volatile int x = 0;}
    if (outer_struct_const.x <= -207) {volatile int keep = 0;}
    if (outer_struct_const.inner_struct.x != 208) {volatile int x = 0;}
    if (-209 == outer_struct_typedef.x) {volatile int keep = 0;}
    if (210 >= outer_struct_typedef.inner_struct.x) {volatile int x = 0;}
}

static void assign_to_member_of_struct_dereferenced_via_pointer
(
    outer_struct_t* outer_struct_ptr,
    outer_struct_t* const outer_struct_ptr_const_1,
    outer_struct_typedef_t* outer_struct_typedef
)
{
    outer_struct_ptr->x = -211;
    outer_struct_ptr->inner_struct.x = 212;
    outer_struct_ptr_const_1->x = -213;
    outer_struct_ptr_const_1->inner_struct.x = 214;
    outer_struct_typedef->x = -215;
    outer_struct_typedef->inner_struct.x = 216;
}

static void compare_with_member_value_of_struct_dereferenced_via_pointer
(
    outer_struct_t* outer_struct_ptr,
    const outer_struct_t* outer_struct_ptr_const_0,
    outer_struct_t* const outer_struct_ptr_const_1,
    outer_struct_typedef_t* outer_struct_typedef
)
{
    if (-217 > outer_struct_ptr->x) {volatile int keep = 0;}
    if (218 < outer_struct_ptr->inner_struct.x) {volatile int x = 0;}
    if (-219 <= outer_struct_ptr_const_0->x) {volatile int keep = 0;}
    if (outer_struct_ptr_const_0->inner_struct.x != 220) {volatile int x = 0;}
    if (outer_struct_ptr_const_1->x == -221) {volatile int keep = 0;}
    if (outer_struct_ptr_const_1->inner_struct.x >= 222) {volatile int x = 0;}
    if (outer_struct_typedef->x > -223) {volatile int keep = 0;}
    if (outer_struct_typedef->inner_struct.x < 224) {volatile int x = 0;}
}

static void receive_integer(int a) {}

static void receive_integer_typedef(typedef_int a) {}

static int return_integer()
{
    return -225;
}

static typedef_int return_integer_typedef()
{
    return 226;
}
