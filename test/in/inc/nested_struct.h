#pragma once

typedef struct
{
    int x;
} inner_struct_t;

typedef struct 
{
    int x;
    inner_struct_t inner_struct;
} outer_struct_t;
