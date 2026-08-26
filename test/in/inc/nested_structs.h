#pragma once

struct inner_struct_t
{
    int x;
};

struct outer_struct_t
{
    int x;
    struct inner_struct_t inner_struct;
};

typedef struct
{
    int x;
} inner_struct_typedef_t;

typedef struct
{
    int x;
    inner_struct_typedef_t inner_struct_typedef;
} outer_struct_typedef_t;
