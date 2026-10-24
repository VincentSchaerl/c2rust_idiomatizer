#pragma once

#include "typedefs.h"

typedef struct
{
    int x;
} inner_struct_t;

typedef struct 
{
    int x;
    inner_struct_t inner_struct;
} outer_struct_t;


typedef struct
{
    typedef_int x;
} inner_struct_typedef_t;

typedef struct 
{
    typedef_int x;
    inner_struct_typedef_t inner_struct;
} outer_struct_typedef_t;
