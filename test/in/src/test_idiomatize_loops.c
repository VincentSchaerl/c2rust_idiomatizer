#include <stdint.h>
#include <stdio.h>

#include "nested_struct.h"

static void receive_integers_for_l
(
    int _int,
    const int _int_const,
    int* _int_ptr,
    const int* _int_ptr_const,
    int array[],
    struct outer_struct_t outer_struct,
    struct outer_struct_t* outer_struct_ptr
);

// T-values

static int _int_global = 0;
static const int _int_global_const = 0;
static int* _int_ptr_global = &_int_global;
static const int* _int_ptr_global_const = &_int_global_const;
static int _int_array_global[10] = {0};
static struct outer_struct_t outer_struct_global = { 0, { 0 } };
static struct outer_struct_t* outer_struct_ptr_global = &outer_struct_global;

void test_idiomatize_loops()
{
    // T-values

    int _int = 0;
    const int _int_const = 0;
    static int _int_static = 0;
    int* _int_ptr = &_int;
    const int* _int_ptr_const = &_int;
    int _int_array[10] = {0};
    struct outer_struct_t outer_struct = { 0, { 0 } };
    struct outer_struct_t* outer_struct_ptr = &outer_struct;
    
    // BASE CASE

    // basic form

    for (int i_0 = 0; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}

    // T

    for (int8_t i_0 = 0; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (int16_t i_0 = 0; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (int32_t i_0 = 0; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (int64_t i_0 = 0; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (uint8_t i_0 = 0; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (uint16_t i_0 = 0; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (uint32_t i_0 = 0; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (uint64_t i_0 = 0; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (signed char i_0 = 0; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (unsigned char i_0 = 0; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (short i_0 = 0; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (unsigned short i_0 = 0; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (int i_0 = 0; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_0 = 0; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (long i_0 = 0; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (unsigned long i_0 = 0; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (long long i_0 = 0; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (unsigned long long i_0 = 0; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (intptr_t i_0 = 0; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (uintptr_t i_0 = 0; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (size_t i_0 = 0; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}

    // L

    for (int i_0 = 0; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (int i_0 = -3; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (int i_0 = 6; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (int i_0 = -011; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (int i_0 = 0xC; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (int i_0 = _int_global; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (int i_0 = _int_global_const; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (int i_0 = _int; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (int i_0 = _int_const; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (int i_0 = _int_static; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (int i_0 = *_int_ptr_global; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (int i_0 = *_int_ptr_global_const; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (int i_0 = *_int_ptr; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (int i_0 = *_int_ptr_const; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    int a = 0;
    for (int i_0 = _int_array_global[0]; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (int i_0 = _int_array_global[a]; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (int i_0 = _int_array[0]; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (int i_0 = _int_array[a]; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (int i_0 = outer_struct_global.x; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (int i_0 = outer_struct_global.inner_struct.x; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (int i_0 = outer_struct.x; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (int i_0 = outer_struct.inner_struct.x; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (int i_0 = outer_struct_ptr_global->x; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (int i_0 = outer_struct_ptr_global->inner_struct.x; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (int i_0 = outer_struct_ptr->x; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (int i_0 = outer_struct_ptr->inner_struct.x; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    receive_integers_for_l(0, 0, _int_ptr, _int_ptr_const, _int_array, outer_struct, outer_struct_ptr);

    // END

    printf("End of test_idiomatize_loops().\n");
}



static void receive_integers_for_l
(
    int _int,
    const int _int_const,
    int* _int_ptr,
    const int* _int_ptr_const,
    int array[],
    struct outer_struct_t outer_struct,
    struct outer_struct_t* outer_struct_ptr
)
{
    for (int i_0 = _int; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (int i_0 = _int_const; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (int i_0 = *_int_ptr; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (int i_0 = *_int_ptr_const; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (int i_0 = array[0]; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    int a = 0;
    for (int i_0 = array[a]; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (int i_0 = outer_struct.x; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (int i_0 = outer_struct.inner_struct.x; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (int i_0 = outer_struct_ptr->x; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
    for (int i_0 = outer_struct_ptr->inner_struct.x; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}
}


























/*
static void receive_integers_for_l
(
    int a,
    int* a_p,
    int array[],
    struct OuterStruct outerStruct,
    struct OuterStruct* outerStruct_p,
    OuterStructTypedef outerStructTypedef,
    OuterStructTypedef* outerStructTypedef_p
);

static void receive_integers_for_n
(
    int a,
    int* a_p,
    int array[],
    struct OuterStruct outerStruct,
    struct OuterStruct* outerStruct_p,
    OuterStructTypedef outerStructTypedef,
    OuterStructTypedef* outerStructTypedef_p
);

static int receive_and_return_integers_for_body
(
    int a,
    int* a_p,
    int array[],
    struct OuterStruct outerStruct,
    struct OuterStruct* outerStruct_p,
    OuterStructTypedef outerStructTypedef,
    OuterStructTypedef* outerStructTypedef_p
);

static int a;
static int array0[10];
static struct OuterStruct outerStruct0;
static OuterStructTypedef outerStructTypedef0;

void fill_arr0(int arr[])
{
    for (int i = 0; i < 10; i++)
    {
        arr[i] = i;
    }
}

void fill_arr1(int arr[10])
{
    for (int i = 0; i < 10; i++)
    {
        arr[i] = i;
    }
}

void fill_arr2(int* arr)
{
    for (int i = 0; i < 10; i++)
    {
        arr[i] = i;
    }
}

void fill_arr3(int arr[])
{
    for (int i = 0; i < 10; i++)
    {
        *(arr + i) = i;
    }
}

void fill_arr4(int arr[10])
{
    for (int i = 0; i < 10; i++)
    {
        *(arr + i) = i;
    }
}

void fill_arr5(int* arr)
{
    for (int i = 0; i < 10; i++)
    {
        *(arr + i) = i;
    }
}

int arr4[5];

void _fill_arr0(int arr[], int length)
{
    for (int i = 0; i < length; i++)
    {
        arr[i] = i;
    }
}

void _fill_arr1(const int arr[], long length)
{
    for (int i = 0; i < length; i++)
    {
    }
}

void _fill_arr2(int arr[const], int length)
{
    for (int i = 0; i < length; i++)
    {
    }
}

void _fill_arr3(const int arr[const], int length)
{
    for (int i = 0; i < length; i++)
    {
    }
}

void _fill_arr4(int arr[], size_t length)
{
    for (int i = 0; i < length; i++)
    {
        arr[i] = i;
    }
}

#define LENGTH_0 5
enum { LENGTH_2 = 5 };

int foo0[5];
static int foo1[5];










































static void receive_integers_for_l
(
    int a,
    int* a_p,
    int array[],
    struct OuterStruct outerStruct,
    struct OuterStruct* outerStruct_p,
    OuterStructTypedef outerStructTypedef,
    OuterStructTypedef* outerStructTypedef_p
)
{
    a = 0;
    *a_p = 0;
    for (int i = a; i < 10; i++) {}
    for (int i = *a_p; i < 10; i++) {}

    a = 0;
    array[0] = 0;
    for (int i = array[0]; i < 10; i++) {}
    for (int i = array[a]; i < 10; i++) {}

    outerStruct.x = 0;
    outerStruct.innerStruct.x = 0;
    outerStruct_p->x = 0;
    outerStruct_p->innerStruct.x = 0;
    outerStructTypedef.x = 0;
    outerStructTypedef.innerStructTypedef.x = 0;
    outerStructTypedef_p->x = 0;
    outerStructTypedef_p->innerStructTypedef.x = 0;
    for (int i = outerStruct.x; i < 10; i++) {}
    for (int i = outerStruct.innerStruct.x; i < 10; i++) {}
    for (int i = outerStruct_p->x; i < 10; i++) {}
    for (int i = outerStruct_p->innerStruct.x; i < 10; i++) {}
    for (int i = outerStructTypedef.x; i < 10; i++) {}
    for (int i = outerStructTypedef.innerStructTypedef.x; i < 10; i++) {}
    for (int i = outerStructTypedef_p->x; i < 10; i++) {}
    for (int i = outerStructTypedef_p->innerStructTypedef.x; i < 10; i++) {}
}

static void receive_integers_for_n
(
    int a,
    int* a_p,
    int array[],
    struct OuterStruct outerStruct,
    struct OuterStruct* outerStruct_p,
    OuterStructTypedef outerStructTypedef,
    OuterStructTypedef* outerStructTypedef_p
)
{
    a = 10;
    *a_p = 10;
    for (int i = 0; i < a; i++) {}
    for (int i = 0; i < *a_p; i++) {}

    a = 0;
    array[0] = 10;
    for (int i = 0; i < array[0]; i++) {}
    for (int i = 0; i < array[a]; i++) {}

    outerStruct.x = 10;
    outerStruct.innerStruct.x = 10;
    outerStruct_p->x = 10;
    outerStruct_p->innerStruct.x = 10;
    outerStructTypedef.x = 10;
    outerStructTypedef.innerStructTypedef.x = 10;
    outerStruct_p->x = 10;
    outerStruct_p->innerStruct.x = 10;
    for (int i = 0; i < outerStruct.x; i++) {}
    for (int i = 0; i < outerStruct.innerStruct.x; i++) {}
    for (int i = 0; i < outerStruct_p->x; i++) {}
    for (int i = 0; i < outerStruct_p->innerStruct.x; i++) {}
    for (int i = 0; i < outerStructTypedef.x; i++) {}
    for (int i = 0; i < outerStructTypedef.innerStructTypedef.x; i++) {}
    for (int i = 0; i < outerStructTypedef_p->x; i++) {}
    for (int i = 0; i < outerStructTypedef_p->innerStructTypedef.x; i++) {}
}

static int receive_and_return_integers_for_body
(
    int a,
    int* a_p,
    int array[],
    struct OuterStruct outerStruct,
    struct OuterStruct* outerStruct_p,
    OuterStructTypedef outerStructTypedef,
    OuterStructTypedef* outerStructTypedef_p
)
{
    for (int i = 0; i < 10; i++)
    {
        a = i;
        *a_p = i;

        array[0] = i;

        outerStruct.x = i;
        outerStruct.innerStruct.x = i;
        outerStruct_p->x = i;
        outerStruct_p->innerStruct.x = i;
        outerStructTypedef.x = i;
        outerStructTypedef.innerStructTypedef.x = i;
        outerStructTypedef_p->x = i;
        outerStructTypedef_p->innerStructTypedef.x = i;

        a = array[i];
        array[i] = a;

        return i;
    }
}












































    short _i = 0;
    for (int i = _i; i < 200; i++) {int x = 1; if (x) break;}
    int foo2[5];
    for (size_t i = 0; i < sizeof(foo2) / sizeof(foo2[0]); i++)
    {
        foo2[i] = i;
    }
    int _test;
    for (int i = 0; i < 10; i++) 
    {
        if (_test) continue;
        printf("Hllo\n");
    }
    for (unsigned int i = 0; i < 10; i++) {}
    unsigned long step = 1;
    for (unsigned long i = 0; i < 10; i += step) {}
    for (uint8_t i = 0; i < 10; i += 2) {}
    for (uint64_t i = 0; i < 10; i -= 1) {}
    for (uintptr_t i = 0; i < 10; i --) {}
    for (size_t i = 0; i < 10; i -= 2) {}

    for (int i = 0; i < 5; i++)
    {
        foo2[i] = i;
    }
    const int foo3[5];
    static int foo4[5];

    int foo5[5];
    _fill_arr0(foo5, 5);
    _fill_arr1(foo5, 5);
    _fill_arr2(foo5, 5);
    _fill_arr3(foo5, 5);

    int foo6[LENGTH_0];
    _fill_arr1(foo6, LENGTH_0);
    for (int i = 0; i < LENGTH_0; i++);

    int foo7[LENGTH_2];
    _fill_arr1(foo7, LENGTH_2);
    for (int i = 0; i < LENGTH_2; i++);

    int foo8[LENGTH_0];
    _fill_arr4(foo8, sizeof(foo8) / sizeof(foo8[0]));
    for (size_t i = 0; i < sizeof(foo8) / sizeof(foo8[0]); i++);





    for (int i = 10; i > 0; i += -2){}

    int arr0[10];
    for (int i = 0; i < sizeof(arr0) / sizeof(arr0[0]); i++){}
    _fill_arr0(arr0, 10);

    size_t si_ze = sizeof(arr0) / sizeof(arr0[0]);

    size_t _length = 10;
    int arr1[_length];

    int* length_p = &_length;
    int arr2[*length_p];
    _fill_arr0(arr2, 10);
    
    int _help[] = {10};
    int arr3[_help[0]];
    _fill_arr0(arr3, _help[0]);

    struct _help2_t {int len;};
    struct _help2_t _help2 = {10};
    int arr4[_help2.len];
    _fill_arr0(arr4, _help2.len);

    struct _help2_t* _help2_p = &_help2;
    int arr5[_help2_p->len];
    _fill_arr0(arr5, _help2_p->len);

    int b;
    int* b_p = &b;
    const int* b_p0 = &b;
    int* const b_p1 = &b;
    const int* const b_p2 = &b;
    int array1[10];
    struct OuterStruct outerStruct1;
    OuterStructTypedef outerStructTypedef1;

    // for (<T> <I> = <L>; <I> <relational operator> <N>; <I><decrementing or incrementing>)
    // {
    //     <body>
    // }

    // base case

    struct OuterStruct outerStruct2;
    const struct OuterStruct outerStruct3;
    struct OuterStruct* outerStruct2_p0 = &outerStruct2;
    const struct OuterStruct* outerStruct2_p1 = &outerStruct2;
    struct OuterStruct* const outerStruct2_p2 = &outerStruct2;
    const struct OuterStruct* const outerStruct2_p3 = &outerStruct2;
    struct OuterStruct* outerStruct3_p4 = &outerStruct3;
    const struct OuterStruct* outerStruct3_p5 = &outerStruct3;
    struct OuterStruct* const outerStruct3_p6 = &outerStruct3;
    const struct OuterStruct* const outerStruct3_p7 = &outerStruct3;

    int arr[10];

    for (int i = 0; i < 10; i++)
    {
        arr[i] = i;
    }

    fill_arr0(arr);
    fill_arr1(arr);
    fill_arr2(arr);
    fill_arr3(arr);
    fill_arr4(arr);
    fill_arr5(arr);

    int _y = 1;

    for (int i = 0; i < 10; i++) {}
    for (int i = 0; i > 10; i--) {}
    for (int i = 0; i < 10; i += 1) {}
    for (int i = 0; i > 10; i -= 1) {}
    for (int i = 0; i < 10; i = i + 1) {}
    for (int i = 0; i > 10; i = i - 1) {}
    for (int i = 0; i < 10; i += _y) {}
    for (int i = 0; i > 10; i -= _y) {}
    for (int i = 0; i < 10; i = i + _y) {}
    for (int i = 0; i > 10; i = i - _y) {}

    struct OuterStruct* outerStruct1_p = &outerStruct1;

    for (int i = 0; i < (*outerStruct1_p).x; i++)
    {
    }
    
    short x = 5;

    for (int i = 0; i < sizeof(arr) / sizeof(arr[0]); i++);

    for (int i = 0; i < 20 / 2; i++);

    int __a = 20;
    int __b = 2;

    for (int i = 0; i < __a / __b; i++);

    for (int i = x; i < 10; i++) {}
    for (int i = 0; i > -10; i--) {}

    int steps = 2;

    for (int i = 0; i < 10; i += 2) {}
    for (int i = 0; i > -10; i -= 2) {}

    for (int i = 0; i < 10; i = i + 2) {}
    for (int i = 0; i > .10; i = i - 2) {}

    for (int i = 0; i < 10; i += steps) {}
    for (int i = 0; i > -10; i -= steps) {}

    for (int i = 0; i < 10; i = i + steps) {}
    for (int i = 0; i > -10; i = i - steps) {}

    // L

    for (int i = -1; i < 10; i++) {}
    for (int i = 0; i < 10; i++) {}
    for (int i = 1; i < 10; i++) {}

    a = 0;
    b = 0;
    for (int i = a; i < 10; i++) {}
    for (int i = b; i < 10; i++) {}

    a = 0;
    array0[0] = 0;
    array1[0] = 0;
    for (int i = array0[0]; i < 10; i++) {}
    for (int i = array0[a]; i < 10; i++) {}
    for (int i = array1[0]; i < 10; i++) {}
    for (int i = array1[a]; i < 10; i++) {}

    outerStruct0.x = 0;
    outerStruct0.innerStruct.x = 0;
    outerStructTypedef0.x = 0;
    outerStructTypedef0.innerStructTypedef.x = 0;
    outerStruct1.x = 0;
    outerStruct1.innerStruct.x = 0;
    outerStructTypedef1.x = 0;
    outerStructTypedef1.innerStructTypedef.x = 0;
    for (int i = outerStruct0.x; i < 10; i++) {}
    for (int i = outerStruct0.innerStruct.x; i < 10; i++) {}
    for (int i = outerStructTypedef0.x; i < 10; i++) {}
    for (int i = outerStructTypedef0.innerStructTypedef.x; i < 10; i++) {}
    for (int i = outerStruct1.x; i < 10; i++) {}
    for (int i = outerStruct1.innerStruct.x; i < 10; i++) {}
    for (int i = outerStructTypedef1.x; i < 10; i++) {}
    for (int i = outerStructTypedef1.innerStructTypedef.x; i < 10; i++) {}

    receive_integers_for_l(a, &a, array0, outerStruct0, &outerStruct0, outerStructTypedef0, &outerStructTypedef0);

    // <relational operator>

    for (int i = 0; i < 10; i++) {}
    for (int i = 0; i <= 10; i++) {}
    for (int i = 0; i > 10; i++) {}
    for (int i = 0; i >= 10; i++) {}

    // N

    for (int i = 0; i < -10; i++) {}
    for (int i = 0; i < 0; i++) {}
    for (int i = 0; i < 10; i++) {}

    a = 10;
    b = 10;
    for (int i = 0; i < a; i++) {}
    for (int i = 0; i < b; i++) {}

    a = 0;
    array0[0] = 10;
    array1[0] = 10;
    for (int i = 0; i < array0[0]; i++) {}
    for (int i = 0; i < array0[a]; i++) {}
    for (int i = 0; i < array1[0]; i++) {}
    for (int i = 0; i < array1[a]; i++) {}

    outerStruct0.x = 10;
    outerStruct0.innerStruct.x = 10;
    outerStructTypedef0.x = 10;
    outerStructTypedef0.innerStructTypedef.x = 10;
    outerStruct1.x = 10;
    outerStruct1.innerStruct.x = 10;
    outerStructTypedef1.x = 10;
    outerStructTypedef1.innerStructTypedef.x = 10;
    for (int i = 0; i < outerStruct0.x; i++) {}
    for (int i = 0; i < outerStruct0.innerStruct.x; i++) {}
    for (int i = 0; i < outerStructTypedef0.x; i++) {}
    for (int i = 0; i < outerStructTypedef0.innerStructTypedef.x; i++) {}
    for (int i = 0; i < outerStruct1.x; i++) {}
    for (int i = 0; i < outerStruct1.innerStruct.x; i++) {}
    for (int i = 0; i < outerStructTypedef1.x; i++) {}
    for (int i = 0; i < outerStructTypedef1.innerStructTypedef.x; i++) {}

    for (int i = 0; i < 10 + 1; i++) {}
    for (int i = 0; i < 10 - 1; i++) {}
    for (int i = 0; i < 10 * 2; i++) {}
    for (int i = 0; i < 10 / 2; i++) {}
    for (int i = 0; i < sizeof(array0) / sizeof(array0[0]); i++) {}
    for (int i = 0; i < sizeof(array1) / sizeof(array1[0]); i++) {}

    receive_integers_for_n(a, &a, array0, outerStruct0, &outerStruct0, outerStructTypedef0, &outerStructTypedef0);

    // decrementing or incrementing

    for (int i = 0; i < 10; i++) {}
    for (int i = 0; i > -10; i--) {}
    for (int i = 0; i < 10; i += 2) {}
    for (int i = 0; i > -10; i -= 2) {}
    for (int i = 0; i < 10; i = i + 3) {}
    for (int i = 0; i > -10; i = i - 3) {}

    // body

    for (int i = 0; i < 10; i++)
    {
        a = i;
        b = i;
        int c = i;
        int d;
        d = i;

        array0[0] = i;
        array1[0] = i;
        int array2[10] = {i};
        int array3[10] = {i, i, i, i, i, i, i, i, i, i};

        outerStruct0.x = i;
        outerStruct0.innerStruct.x = i;
        outerStructTypedef0.x = i;
        outerStructTypedef0.innerStructTypedef.x = i;
        outerStruct1.x = i;
        outerStruct1.innerStruct.x = i;
        outerStructTypedef1.x = i;
        outerStructTypedef1.innerStructTypedef.x = i;
        struct OuterStruct outerStruct2 = { i, { i } };
        OuterStructTypedef outerStructTypedef2 = { i, { i } };
        struct OuterStruct outerStruct3 = { .x = i, .innerStruct = { .x = i } };
        OuterStructTypedef outerStructTypedef3 = { .x = i, .innerStructTypedef = { .x = i } };

        a = array0[i];
        array0[i] = a;
        a = array1[i];
        array1[i] = a;
        struct OuterStruct outerStructs[10];
        outerStructs[i].x = a;
        outerStructs[i].innerStruct.x = a;

        a = b + i;
        a = i - b;
        a = b * i;
        b = 1;
        a = i % b;

        if (i) {};
        if (!i) {};
        if (i > 0) {};

        int* p = array0 + i;
        a = *(array0 + i);

        switch (i)
        {
            case 1:
                break;
        }

        a = (long)i;
        a = i > 0 ? i : -i;
        char str[100];
        snprintf(str, sizeof(str), "Value of i: %d\n", i);
        sizeof(i);

        receive_and_return_integers_for_body
        (
            i,
            &a,
            (int[10]){i},
            (struct OuterStruct){ i, { i } },
            &outerStruct0,
            (OuterStructTypedef){ i, { i } },
            &outerStructTypedef0
        );
        receive_and_return_integers_for_body
        (
            i, 
            &a, 
            (int[]){i, i, i, i, i, i, i, i, i, i}, 
            (struct OuterStruct){ .x = i, .innerStruct = { .x = i } },
            &outerStruct0, 
            (OuterStructTypedef){ .x = i, .innerStructTypedef = { .x = i } },
            &outerStructTypedef0
        );
    }

    // index type to usize

    for (int i = 0; i < sizeof(array0) / sizeof(array0[0]); i++)
    {
        array0[i] = 0;
    }

    // index based to iterator based

    for (int i = 0; i < sizeof(array0) / sizeof(array0[0]); i++)
    {
        array0[i] = 0;
    }

    for (int i = 0; i < sizeof(array0) / sizeof(array0[0]); i++)
    {
        array0[i] = i;
    }
*/
