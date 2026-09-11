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
