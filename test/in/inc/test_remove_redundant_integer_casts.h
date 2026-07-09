#pragma once

#include "nested_structs.h"

void test_remove_redundant_integer_casts();
void assign_to_variable(int a);
void assign_to_memory_location(int* a_p);
void assign_to_array_element(int array[]);
void assign_to_struct_member(struct OuterStruct outerStruct, OuterStructTypedef outerStructTypedef);
void assign_to_member_of_struct_dereferenced_via_a_pointer
(
    struct OuterStruct* outerStruct_p,
    OuterStructTypedef* outerStructTypedef_p
);
void receive_integer(int a);
int return_integer();
