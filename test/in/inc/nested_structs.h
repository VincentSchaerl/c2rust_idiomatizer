#pragma once

struct InnerStruct
{
    int x;
};

struct OuterStruct
{
    int x;
    struct InnerStruct innerStruct;
};

typedef struct
{
    int x;
} InnerStructTypedef;

typedef struct
{
    int x;
    InnerStructTypedef innerStructTypedef;
} OuterStructTypedef;
