#pragma once

struct InnerStruct
{
    int i;
};

struct OuterStruct
{
    int i;
    struct InnerStruct innerStruct;
};

typedef struct
{
    int i;
} InnerStructTypedef;

typedef struct
{
    int i;
    InnerStructTypedef innerStructTypedef;
} OuterStructTypedef;
