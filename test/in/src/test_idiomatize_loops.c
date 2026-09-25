#include <stdint.h>
#include <stdio.h>

#include "nested_struct.h"

static void receive_t_values_for_l
(
    int variable,
    const int variable_const,
    int* pointer,
    const int* pointer_const_0,
    int* const pointer_const_1,
    outer_struct_t outer_struct,
    const outer_struct_t outer_struct_const,
    outer_struct_t* outer_struct_ptr,
    const outer_struct_t* outer_struct_ptr_const_0,
    outer_struct_t* const outer_struct_ptr_const_1
);

static void receive_stable_t_values_for_n
(
    int stable_variable,
    const int stable_variable_const,
    outer_struct_t stable_outer_struct,
    const outer_struct_t stable_outer_struct_const
);

static void receive_statically_determinable_t_values_for_n
(
    int stable_variable,
    outer_struct_t stable_outer_struct
);

// T-values

extern int variable_global;
#define VARIABLE_GLOBAL_CONST 0     // also stable
static int variable_global_static = 0;

extern int* pointer_global;
extern const int* pointer_global_const_0;
static int* pointer_global_static = &variable_global_static;

extern int array_global[10];
static int array_global_static[10] = {0};

extern outer_struct_t outer_struct_global;
static outer_struct_t outer_struct_global_static = { 0, { 0 } };

extern outer_struct_t* outer_struct_ptr_global;
extern const outer_struct_t* outer_struct_ptr_global_const_0;
static outer_struct_t* outer_struct_ptr_global_static = &outer_struct_global;

void test_idiomatize_loops()
{
    // T-values

    int variable = 0;
    const int variable_const = 0;
    static int variable_static = 0;

    int* pointer = &variable;
    const int* pointer_const_0 = &variable_const;
    int* const pointer_const_1 = &variable;
    static int* pointer_static = &variable_static;

    int array[10] = {0};
    const int array_const[10] = {0};
    static int array_static[10] = {0};

    outer_struct_t outer_struct = { 0, { 0 } };
    const outer_struct_t outer_struct_const = { 0, { 0 } };
    static outer_struct_t outer_struct_static = { 0, { 0 } };

    outer_struct_t* outer_struct_ptr = &outer_struct;
    const outer_struct_t* outer_struct_ptr_const_0 = &outer_struct_const;
    outer_struct_t* const outer_struct_ptr_const_1 = &outer_struct;
    static outer_struct_t* outer_struct_ptr_static = &outer_struct_static;

    // stable T-values

    int stable_variable = 0;
    const int stable_variable_const = 0;

    int stable_array[10] = {0};
    const int stable_array_const[10] = {0};
    static int stable_array_static[10] = {0};

    outer_struct_t stable_outer_struct = { 0, { 0 } };
    const outer_struct_t stable_outer_struct_const = { 0, { 0 } };
    static outer_struct_t stable_outer_struct_static = { 0, { 0 } };

    // (stable) U-values

    signed char _signed_char = 0;
    unsigned char _unsigned_char = 0;
    short _short = 0;
    unsigned short _unsigned_short = 0;
    unsigned int _unsigned_int = 0;
    long _long = 0;
    unsigned long _unsigned_long = 0;
    long long _long_long = 0;
    unsigned long long _unsigned_long_long = 0;
    int8_t _int8_t = 0;
    uint8_t _uint8_t = 0;
    int16_t _int16_t = 0;
    uint16_t _uint16_t = 0;
    uint32_t _uint32_t = 0;
    int64_t _int64_t = 0;
    uint64_t _uint64_t = 0;
    intptr_t _intptr_t = 0;
    uintptr_t _uintptr_t = 0;
    size_t _size_t = 0;

    // BASE CASE

    // basic form

    for (int i_0 = 0; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}

    for (int f_0 = 0; f_0 < 10; f_0++) {f_0++; volatile int keep = 1; if (keep) break;}
    for (int f_1 = 0; f_1 < 10; f_1++) {int* ptr = &f_1; volatile int keep = 1; if (keep) break;}

    // T

    for (int i_1 = 0; i_1 < 10; i_1++) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_2 = 0; i_2 < 10; i_2++) {volatile int keep = 1; if (keep) break;}
    for (long i_3 = 0; i_3 < 10; i_3++) {volatile int keep = 1; if (keep) break;}
    for (unsigned long i_4 = 0; i_4 < 10; i_4++) {volatile int keep = 1; if (keep) break;}
    for (long long i_5 = 0; i_5 < 10; i_5++) {volatile int keep = 1; if (keep) break;}
    for (unsigned long long i_6 = 0; i_6 < 10; i_6++) {volatile int keep = 1; if (keep) break;}
    for (int32_t i_7 = 0; i_7 < 10; i_7++) {volatile int keep = 1; if (keep) break;}
    for (uint32_t i_8 = 0; i_8 < 10; i_8++) {volatile int keep = 1; if (keep) break;}
    for (int64_t i_9 = 0; i_9 < 10; i_9++) {volatile int keep = 1; if (keep) break;}
    for (uint64_t i_10 = 0; i_10 < 10; i_10++) {volatile int keep = 1; if (keep) break;}
    for (size_t i_11 = 0; i_11 < 10; i_11++) {volatile int keep = 1; if (keep) break;}

    for (char f_2 = 0; f_2 < 10; f_2++) {volatile int keep = 1; if (keep) break;}

    // L

    for (int i_12 = 0; i_12 < 10; i_12++) {volatile int keep = 1; if (keep) break;}
    for (int i_13 = -3; i_13 < 10; i_13++) {volatile int keep = 1; if (keep) break;}
    for (int i_14 = 6; i_14 < 10; i_14++) {volatile int keep = 1; if (keep) break;}
    for (int i_15 = -011; i_15 < 10; i_15++) {volatile int keep = 1; if (keep) break;}
    for (int i_16 = 0xC; i_16 < 10; i_16++) {volatile int keep = 1; if (keep) break;}

    for (int i_17 = variable_global; i_17 < 10; i_17++) {volatile int keep = 1; if (keep) break;}
    for (int i_18 = VARIABLE_GLOBAL_CONST; i_18 < 10; i_18++) {volatile int keep = 1; if (keep) break;}
    for (int i_19 = variable_global_static; i_19 < 10; i_19++) {volatile int keep = 1; if (keep) break;}
    for (int i_20 = variable; i_20 < 10; i_20++) {volatile int keep = 1; if (keep) break;}
    for (int i_21 = variable_const; i_21 < 10; i_21++) {volatile int keep = 1; if (keep) break;}
    for (int i_22 = variable_static; i_22 < 10; i_22++) {volatile int keep = 1; if (keep) break;}

    for (int i_23 = *pointer_global; i_23 < 10; i_23++) {volatile int keep = 1; if (keep) break;}
    for (int i_24 = *pointer_global_const_0; i_24 < 10; i_24++) {volatile int keep = 1; if (keep) break;}
    for (int i_25 = *pointer_global_static; i_25 < 10; i_25++) {volatile int keep = 1; if (keep) break;}
    for (int i_26 = *pointer; i_26 < 10; i_26++) {volatile int keep = 1; if (keep) break;}
    for (int i_27 = *pointer_const_0; i_27 < 10; i_27++) {volatile int keep = 1; if (keep) break;}
    for (int i_28 = *pointer_const_1; i_28 < 10; i_28++) {volatile int keep = 1; if (keep) break;}
    for (int i_29 = *pointer_static; i_29 < 10; i_29++) {volatile int keep = 1; if (keep) break;}

    const int i = 0;
    for (int i_30 = array_global[0]; i_30 < 10; i_30++) {volatile int keep = 1; if (keep) break;}
    for (int i_31 = array_global[i]; i_31 < 10; i_31++) {volatile int keep = 1; if (keep) break;}
    for (int i_32 = array_global_static[0]; i_32 < 10; i_32++) {volatile int keep = 1; if (keep) break;}
    for (int i_33 = array_global_static[i]; i_33 < 10; i_33++) {volatile int keep = 1; if (keep) break;}
    for (int i_34 = array[0]; i_34 < 10; i_34++) {volatile int keep = 1; if (keep) break;}
    for (int i_35 = array[i]; i_35 < 10; i_35++) {volatile int keep = 1; if (keep) break;}
    for (int i_36 = array_const[0]; i_36 < 10; i_36++) {volatile int keep = 1; if (keep) break;}
    for (int i_37 = array_const[i]; i_37 < 10; i_37++) {volatile int keep = 1; if (keep) break;}
    for (int i_38 = array_static[0]; i_38 < 10; i_38++) {volatile int keep = 1; if (keep) break;}
    for (int i_39 = array_static[i]; i_39 < 10; i_39++) {volatile int keep = 1; if (keep) break;}

    for (int i_40 = outer_struct_global.x; i_40 < 10; i_40++) {volatile int keep = 1; if (keep) break;}
    for (int i_41 = outer_struct_global.inner_struct.x; i_41 < 10; i_41++) {volatile int keep = 1; if (keep) break;}
    for (int i_42 = outer_struct_global_static.x; i_42 < 10; i_42++) {volatile int keep = 1; if (keep) break;}
    for (int i_43 = outer_struct_global_static.inner_struct.x; i_43 < 10; i_43++) {volatile int keep = 1; if (keep) break;}
    for (int i_44 = outer_struct.x; i_44 < 10; i_44++) {volatile int keep = 1; if (keep) break;}
    for (int i_45 = outer_struct.inner_struct.x; i_45 < 10; i_45++) {volatile int keep = 1; if (keep) break;}
    for (int i_46 = outer_struct_const.x; i_46 < 10; i_46++) {volatile int keep = 1; if (keep) break;}
    for (int i_47 = outer_struct_const.inner_struct.x; i_47 < 10; i_47++) {volatile int keep = 1; if (keep) break;}
    for (int i_48 = outer_struct_static.x; i_48 < 10; i_48++) {volatile int keep = 1; if (keep) break;}
    for (int i_49 = outer_struct_static.inner_struct.x; i_49 < 10; i_49++) {volatile int keep = 1; if (keep) break;}

    for (int i_50 = outer_struct_ptr_global->x; i_50 < 10; i_50++) {volatile int keep = 1; if (keep) break;}
    for (int i_51 = outer_struct_ptr_global->inner_struct.x; i_51 < 10; i_51++) {volatile int keep = 1; if (keep) break;}
    for (int i_52 = outer_struct_ptr_global_const_0->x; i_52 < 10; i_52++) {volatile int keep = 1; if (keep) break;}
    for (int i_53 = outer_struct_ptr_global_const_0->inner_struct.x; i_53 < 10; i_53++) {volatile int keep = 1; if (keep) break;}
    for (int i_54 = outer_struct_ptr_global_static->x; i_54 < 10; i_54++) {volatile int keep = 1; if (keep) break;}
    for (int i_55 = outer_struct_ptr_global_static->inner_struct.x; i_55 < 10; i_55++) {volatile int keep = 1; if (keep) break;}
    for (int i_56 = outer_struct_ptr->x; i_56 < 10; i_56++) {volatile int keep = 1; if (keep) break;}
    for (int i_57 = outer_struct_ptr->inner_struct.x; i_57 < 10; i_57++) {volatile int keep = 1; if (keep) break;}
    for (int i_58 = outer_struct_ptr_const_0->x; i_58 < 10; i_58++) {volatile int keep = 1; if (keep) break;}
    for (int i_59 = outer_struct_ptr_const_0->inner_struct.x; i_59 < 10; i_59++) {volatile int keep = 1; if (keep) break;}
    for (int i_60 = outer_struct_ptr_const_1->x; i_60 < 10; i_60++) {volatile int keep = 1; if (keep) break;}
    for (int i_61 = outer_struct_ptr_const_1->inner_struct.x; i_61 < 10; i_61++) {volatile int keep = 1; if (keep) break;}
    for (int i_62 = outer_struct_ptr_static->x; i_62 < 10; i_62++) {volatile int keep = 1; if (keep) break;}
    for (int i_63 = outer_struct_ptr_static->inner_struct.x; i_63 < 10; i_63++) {volatile int keep = 1; if (keep) break;}

    receive_t_values_for_l
    (
        variable,
        variable_const,
        pointer,
        pointer_const_0,
        pointer_const_1,
        outer_struct,
        outer_struct_const,
        outer_struct_ptr,
        outer_struct_ptr_const_0,
        outer_struct_ptr_const_1
    );

    for (int i_64 = _signed_char; i_64 < 10; i_64++) {volatile int keep = 1; if (keep) break;}
    for (int i_65 = _unsigned_char; i_65 < 10; i_65++) {volatile int keep = 1; if (keep) break;}
    for (int i_66 = _short; i_66 < 10; i_66++) {volatile int keep = 1; if (keep) break;}
    for (int i_67 = _unsigned_short; i_67 < 10; i_67++) {volatile int keep = 1; if (keep) break;}
    for (int i_68 = _long_long; i_68 < 10; i_68++) {volatile int keep = 1; if (keep) break;}
    for (int i_69 = _unsigned_long_long; i_69 < 10; i_69++) {volatile int keep = 1; if (keep) break;}
    for (int i_70 = _int8_t; i_70 < 10; i_70++) {volatile int keep = 1; if (keep) break;}
    for (int i_71 = _uint8_t; i_71 < 10; i_71++) {volatile int keep = 1; if (keep) break;}
    for (int i_72 = _int16_t; i_72 < 10; i_72++) {volatile int keep = 1; if (keep) break;}
    for (int i_73 = _uint16_t; i_73 < 10; i_73++) {volatile int keep = 1; if (keep) break;}
    for (int i_74 = _uint32_t; i_74 < 10; i_74++) {volatile int keep = 1; if (keep) break;}
    for (int i_75 = _int64_t; i_75 < 10; i_75++) {volatile int keep = 1; if (keep) break;}
    for (int i_76 = _uint64_t; i_76 < 10; i_76++) {volatile int keep = 1; if (keep) break;}
    for (int i_77 = _intptr_t; i_77 < 10; i_77++) {volatile int keep = 1; if (keep) break;}
    for (int i_78 = _uintptr_t; i_78 < 10; i_78++) {volatile int keep = 1; if (keep) break;}
    for (int i_79 = _size_t; i_79 < 10; i_79++) {volatile int keep = 1; if (keep) break;}

    // relational_operator

    for (int i_80 = 0; i_80 < 10; i_80++) {volatile int keep = 1; if (keep) break;}
    for (int i_81 = 0; i_81 <= 10; i_81++) {volatile int keep = 1; if (keep) break;}
    for (int i_82 = 0; i_82 > 10; i_82--) {volatile int keep = 1; if (keep) break;}
    for (int i_83 = 0; i_83 >= 10; i_83--) {volatile int keep = 1; if (keep) break;}
    for (int i_84 = 0; 10 < i_84; i_84--) {volatile int keep = 1; if (keep) break;}
    for (int i_85 = 0; 10 <= i_85; i_85--) {volatile int keep = 1; if (keep) break;}
    for (int i_86 = 0; 10 > i_86; i_86++) {volatile int keep = 1; if (keep) break;}
    for (int i_87 = 0; 10 >= i_87; i_87++) {volatile int keep = 1; if (keep) break;}

    for (int f_3 = 0; f_3 != 10; f_3++) {volatile int keep = 1; if (keep) break;}

    // N

    for (int i_88 = 0; i_88 < 0; i_88++) {volatile int keep = 1; if (keep) break;}
    for (int i_89 = 0; i_89 < -3; i_89++) {volatile int keep = 1; if (keep) break;}
    for (int i_90 = 0; i_90 < 6; i_90++) {volatile int keep = 1; if (keep) break;}
    for (int i_91 = 0; i_91 < -011; i_91++) {volatile int keep = 1; if (keep) break;}
    for (int i_92 = 0; i_92 < 0xC; i_92++) {volatile int keep = 1; if (keep) break;}

    for (int i_93 = 0; i_93 < VARIABLE_GLOBAL_CONST; i_93++) {volatile int keep = 1; if (keep) break;}
    for (int i_94 = 0; i_94 < stable_variable; i_94++) {volatile int keep = 1; if (keep) break;}
    for (int i_95 = 0; i_95 < stable_variable_const; i_95++) {volatile int keep = 1; if (keep) break;}
    int stable_variable_temp_0 = 0;
    for (int i_96 = 0; i_96 < stable_variable_temp_0; i_96++) {volatile int keep = 1; if (keep) break;}
    int* stable_variable_temp_0_ptr = &stable_variable_temp_0;

    for (int i_97 = 0; i_97 < stable_array[0]; i_97++) {volatile int keep = 1; if (keep) break;}
    for (int i_98 = 0; i_98 < stable_array_const[0]; i_98++) {volatile int keep = 1; if (keep) break;}
    for (int i_99 = 0; i_99 < stable_array_static[0]; i_99++) {volatile int keep = 1; if (keep) break;}
    int stable_array_temp_0[10] = {0};
    int stable_array_temp_1[10] = {0};
    for (int i_100 = 0; i_100 < stable_array_temp_0[0]; i_100++) {volatile int keep = 1; if (keep) break;}
    for (int i_101 = 0; i_101 < stable_array_temp_1[0]; i_101++) {volatile int keep = 1; if (keep) break;}
    int* stable_array_temp_0_ptr = &stable_array_temp_0[1];
    int (*stable_array_temp_1_ptr)[10] = &stable_array_temp_1;

    for (int i_102 = 0; i_102 < stable_outer_struct.x; i_102++) {volatile int keep = 1; if (keep) break;}
    for (int i_103 = 0; i_103 < stable_outer_struct.inner_struct.x; i_103++) {volatile int keep = 1; if (keep) break;}
    for (int i_104 = 0; i_104 < stable_outer_struct_const.x; i_104++) {volatile int keep = 1; if (keep) break;}
    for (int i_105 = 0; i_105 < stable_outer_struct_const.inner_struct.x; i_105++) {volatile int keep = 1; if (keep) break;}
    for (int i_106 = 0; i_106 < stable_outer_struct_static.x; i_106++) {volatile int keep = 1; if (keep) break;}
    for (int i_107 = 0; i_107 < stable_outer_struct_static.inner_struct.x; i_107++) {volatile int keep = 1; if (keep) break;}
    outer_struct_t stable_outer_struct_temp_0 = { 0, { 0 } };
    outer_struct_t stable_outer_struct_temp_1 = { 0, { 0 } };
    for (int i_108 = 0; i_108 < stable_outer_struct_temp_0.x; i_108++) {volatile int keep = 1; if (keep) break;}
    for (int i_109 = 0; i_109 < stable_outer_struct_temp_1.inner_struct.x; i_109++) {volatile int keep = 1; if (keep) break;}
    int* stable_outer_struct_temp_0_ptr = &stable_outer_struct_temp_0.inner_struct.x;
    outer_struct_t* stable_outer_struct_temp_1_ptr = &stable_outer_struct_temp_1;

    for (int32_t i_110 = 0; i_110 < INT16_MAX; i_110++) {volatile int keep = 1; if (keep) break;}
    for (int32_t i_111 = 0; INT16_MAX > i_111; i_111++) {volatile int keep = 1; if (keep) break;}

    receive_stable_t_values_for_n
    (
        stable_variable,
        stable_variable_const,
        stable_outer_struct,
        stable_outer_struct_const
    );

    for (int i_112 = 0; i_112 < _signed_char; i_112++) {volatile int keep = 1; if (keep) break;}
    for (int i_113 = 0; i_113 < _unsigned_char; i_113++) {volatile int keep = 1; if (keep) break;}
    for (int i_114 = 0; i_114 < _short; i_114++) {volatile int keep = 1; if (keep) break;}
    for (int i_115 = 0; i_115 < _unsigned_short; i_115++) {volatile int keep = 1; if (keep) break;}
    for (int i_116 = 0; i_116 < _int8_t; i_116++) {volatile int keep = 1; if (keep) break;}
    for (int i_117 = 0; i_117 < _uint8_t; i_117++) {volatile int keep = 1; if (keep) break;}
    for (int i_118 = 0; i_118 < _int16_t; i_118++) {volatile int keep = 1; if (keep) break;}
    for (int i_119 = 0; i_119 < _uint16_t; i_119++) {volatile int keep = 1; if (keep) break;}

    for (int i_120 = 0; i_120 > 10; i_120--) {volatile int keep = 1; if (keep) break;}
    for (int i_121 = 0; VARIABLE_GLOBAL_CONST < i_121; i_121--) {volatile int keep = 1; if (keep) break;}
    for (int i_122 = 0; i_122 > stable_variable; i_122--) {volatile int keep = 1; if (keep) break;}
    for (int i_123 = 0; stable_array[0] < i_123; i_123--) {volatile int keep = 1; if (keep) break;}
    for (int i_124 = 0; i_124 > stable_outer_struct.x; i_124--) {volatile int keep = 1; if (keep) break;}

    receive_statically_determinable_t_values_for_n
    (
        stable_variable,
        stable_outer_struct
    );

    for (int f_4 = 0; f_4 < variable; f_4++) {volatile int keep = 1; if (keep) break;}
    int variable_temp_0 = 0;
    for (int f_5 = 0; f_5 < variable_temp_0; f_5++)
    {
        int* variable_temp_0_ptr = &variable_temp_0;
        volatile int keep = 1;
        if (keep) break;
    }
    for (int f_6 = 0; f_6 < stable_variable; f_6++) {stable_variable = 0; volatile int keep = 1; if (keep) break;}

    for (int f_7 = 0; f_7 < stable_array[i]; f_7++) {volatile int keep = 1; if (keep) break;}
    int array_temp_0[10] = {0};
    int array_temp_1[10] = {0};
    int* array_temp_0_ptr = &array_temp_0[1];
    int (*array_temp_1_ptr)[10] = &array_temp_1;
    for (int f_8 = 0; f_8 < array_temp_0[0]; f_8++) {volatile int keep = 1; if (keep) break;}
    for (int f_9 = 0; f_9 < array_temp_1[0]; f_9++) {volatile int keep = 1; if (keep) break;}
    int array_temp_2[10] = {0};
    int array_temp_3[10] = {0};
    for (int f_10 = 0; f_10 < array_temp_2[0]; f_10++)
    {
        int* array_temp_2_ptr = &array_temp_2[1];
        volatile int keep = 1;
        if (keep) break;
    }
    for (int f_11 = 0; f_11 < array_temp_3[0]; f_11++)
    {
        int (*array_temp_3_ptr)[10] = &array_temp_3;
        volatile int keep = 1;
        if (keep) break;
    }
    for (int f_12 = 0; f_12 < stable_array[0]; f_12++) {stable_array[1] = 0; volatile int keep = 1; if (keep) break;}

    for (int f_13 = 0; f_13 < outer_struct.x; f_13++) {volatile int keep = 1; if (keep) break;}
    outer_struct_t outer_struct_temp_0 = { 0, { 0 } };
    int* outer_struct_temp_0_ptr = &outer_struct_temp_0.inner_struct.x;
    for (int f_14 = 0; f_14 < outer_struct_temp_0.x; f_14++) {volatile int keep = 1; if (keep) break;}
    outer_struct_t outer_struct_temp_1 = { 0, { 0 } };
    outer_struct_t outer_struct_temp_2 = { 0, { 0 } };
    for (int f_15 = 0; f_15 < outer_struct_temp_1.x; f_15++)
    {
        int* outer_struct_temp_1_ptr = &outer_struct_temp_1.inner_struct.x;
        volatile int keep = 1;
        if (keep) break;
    }
    for (int f_16 = 0; f_16 < outer_struct_temp_2.inner_struct.x; f_16++)
    {
        outer_struct_t* outer_struct_temp_2_ptr = &outer_struct_temp_2;
        volatile int keep = 1;
        if (keep) break;
    }
    outer_struct_t outer_struct_temp_3 = { 0, { 0 } };
    outer_struct_t outer_struct_temp_4 = { 0, { 0 } };
    outer_struct_t outer_struct_temp_5 = { 0, { 0 } };
    for (int f_17 = 0; f_17 < outer_struct_temp_3.x; f_17++)
    {
        outer_struct_temp_3 = (outer_struct_t){ 0, { 0 } };
        volatile int keep = 1;
        if (keep) break;
    }
    for (int f_18 = 0; f_18 < outer_struct_temp_4.x; f_18++)
    {
        outer_struct_temp_4.inner_struct.x = 0;
        volatile int keep = 1;
        if (keep) break;
    }
    for (int f_19 = 0; f_19 < outer_struct_temp_5.inner_struct.x; f_19++)
    {
        outer_struct_temp_5.x = 0;
        volatile int keep = 1;
        if (keep) break;
    }

    for (uint32_t f_20 = 0; f_20 > UINT32_C(4294967295); f_20--) {volatile int keep = 1; if (keep) break;}
    for (int64_t f_21 = 0; INT64_MAX < f_21; f_21--) {volatile int keep = 1; if (keep) break;}
    const uint64_t uint64_max = UINT64_C(18446744073709551615);
    for (uint64_t f_22 = 0; uint64_max < f_22; f_22--) {volatile int keep = 1; if (keep) break;}

    // decrementing_or_incrementing

    for (int i_125 = 0; i_125 < 10; i_125++) {volatile int keep = 1; if (keep) break;}
    for (int i_126 = 0; i_126 <= 10; i_126++) {volatile int keep = 1; if (keep) break;}
    for (int i_127 = 0; 10 > i_127; i_127++) {volatile int keep = 1; if (keep) break;}
    for (int i_128 = 0; 10 >= i_128; i_128++) {volatile int keep = 1; if (keep) break;}

    for (int f_23 = 0; 10 <= f_23; f_23++) {volatile int keep = 1; if (keep) break;}

    for (int i_129 = 0; i_129 > 10; i_129--) {volatile int keep = 1; if (keep) break;}
    for (int i_130 = 0; i_130 >= 10; i_130--) {volatile int keep = 1; if (keep) break;}
    for (int i_131 = 0; 10 < i_131 ; i_131--) {volatile int keep = 1; if (keep) break;}
    for (int i_132 = 0; 10 <= i_132; i_132--) {volatile int keep = 1; if (keep) break;}

    for (int f_24 = 0; 10 >= f_24; f_24--) {volatile int keep = 1; if (keep) break;}

    for (int i_133 = 0; i_133 < 10; i_133 += 1) {volatile int keep = 1; if (keep) break;}
    for (int i_134 = 0; i_134 <= 10; i_134 += 1) {volatile int keep = 1; if (keep) break;}
    for (int i_135 = 0; 10 > i_135; i_135 += 1) {volatile int keep = 1; if (keep) break;}
    for (int i_136 = 0; 10 >= i_136; i_136 += 1) {volatile int keep = 1; if (keep) break;}

    for (int f_25 = 0; 10 < f_25; f_25 += 1) {volatile int keep = 1; if (keep) break;}

    for (int i_137 = 0; i_137 > 10; i_137 -= 1) {volatile int keep = 1; if (keep) break;}
    for (int i_138 = 0; i_138 >= 10; i_138 -= 1) {volatile int keep = 1; if (keep) break;}
    for (int i_139 = 0; 10 < i_139; i_139 -= 1) {volatile int keep = 1; if (keep) break;}
    for (int i_140 = 0; 10 <= i_140; i_140 -= 1) {volatile int keep = 1; if (keep) break;}

    for (int f_26 = 0; 10 > f_26; f_26 -= 1) {volatile int keep = 1; if (keep) break;}

    for (int i_141 = 0; i_141 > 10; i_141 += -1) {volatile int keep = 1; if (keep) break;}
    for (int i_142 = 0; i_142 >= 10; i_142 += -1) {volatile int keep = 1; if (keep) break;}
    for (int i_143 = 0; 10 < i_143; i_143 += -1) {volatile int keep = 1; if (keep) break;}
    for (int i_144 = 0; 10 <= i_144; i_144 += -1) {volatile int keep = 1; if (keep) break;}

    for (int f_27 = 0; 10 >= f_27; f_27 += -1) {volatile int keep = 1; if (keep) break;}

    for (int i_145 = 0; i_145 < 10; i_145 -= -1) {volatile int keep = 1; if (keep) break;}
    for (int i_146 = 0; i_146 <= 10; i_146 -= -1) {volatile int keep = 1; if (keep) break;}
    for (int i_147 = 0; 10 > i_147; i_147 -= -1) {volatile int keep = 1; if (keep) break;}
    for (int i_148 = 0; 10 >= i_148; i_148 -= -1) {volatile int keep = 1; if (keep) break;}

    for (int f_28 = 0; 10 <= f_28; f_28 -= -1) {volatile int keep = 1; if (keep) break;}

    for (int i_149 = 0; i_149 < 10; i_149 = i_149 + 1) {volatile int keep = 1; if (keep) break;}
    for (int i_150 = 0; i_150 <= 10; i_150 = i_150 + 1) {volatile int keep = 1; if (keep) break;}
    for (int i_151 = 0; 10 > i_151; i_151 = i_151 + 1) {volatile int keep = 1; if (keep) break;}
    for (int i_152 = 0; 10 >= i_152; i_152 = i_152 + 1) {volatile int keep = 1; if (keep) break;}

    for (int f_29 = 0; 10 < f_29; f_29 = f_29 + 1) {volatile int keep = 1; if (keep) break;}

    for (int i_153 = 0; i_153 > 10; i_153 = i_153 - 1) {volatile int keep = 1; if (keep) break;}
    for (int i_154 = 0; i_154 >= 10; i_154 = i_154 - 1) {volatile int keep = 1; if (keep) break;}
    for (int i_155 = 0; 10 < i_155; i_155 = i_155 - 1) {volatile int keep = 1; if (keep) break;}
    for (int i_156 = 0; 10 <= i_156; i_156 = i_156 - 1) {volatile int keep = 1; if (keep) break;}

    for (int f_30 = 0; 10 > f_30; f_30 = f_30 - 1) {volatile int keep = 1; if (keep) break;}

    for (int i_157 = 0; i_157 > 10; i_157 = i_157 + -1) {volatile int keep = 1; if (keep) break;}
    for (int i_158 = 0; i_158 >= 10; i_158 = i_158 + -1) {volatile int keep = 1; if (keep) break;}
    for (int i_159 = 0; 10 < i_159; i_159 = i_159 + -1) {volatile int keep = 1; if (keep) break;}
    for (int i_160 = 0; 10 <= i_160; i_160 = i_160 + -1) {volatile int keep = 1; if (keep) break;}

    for (int f_31 = 0; 10 >= f_31; f_31 = f_31 + -1) {volatile int keep = 1; if (keep) break;}

    for (int i_161 = 0; i_161 < 10; i_161 = i_161 - -1) {volatile int keep = 1; if (keep) break;}
    for (int i_162 = 0; i_162 <= 10; i_162 = i_162 - -1) {volatile int keep = 1; if (keep) break;}
    for (int i_163 = 0; 10 > i_163; i_163 = i_163 - -1) {volatile int keep = 1; if (keep) break;}
    for (int i_164 = 0; 10 >= i_164; i_164 = i_164 - -1) {volatile int keep = 1; if (keep) break;}

    for (int f_32 = 0; 10 <= f_32; f_32 = f_32 - -1) {volatile int keep = 1; if (keep) break;}

    for (int i_165 = 0; i_165 < 10; i_165 = 1 + i_165) {volatile int keep = 1; if (keep) break;}
    for (int i_166 = 0; i_166 <= 10; i_166 = 1 + i_166) {volatile int keep = 1; if (keep) break;}
    for (int i_167 = 0; 10 > i_167; i_167 = 1 + i_167) {volatile int keep = 1; if (keep) break;}
    for (int i_168 = 0; 10 >= i_168; i_168 = 1 + i_168) {volatile int keep = 1; if (keep) break;}

    for (int f_33 = 0; 10 < f_33; f_33 = 1 + f_33) {volatile int keep = 1; if (keep) break;}

    for (int i_169 = 0; i_169 > 10; i_169 = -1 + i_169) {volatile int keep = 1; if (keep) break;}
    for (int i_170 = 0; i_170 >= 10; i_170 = -1 + i_170) {volatile int keep = 1; if (keep) break;}
    for (int i_171 = 0; 10 < i_171; i_171 = -1 + i_171) {volatile int keep = 1; if (keep) break;}
    for (int i_172 = 0; 10 <= i_172; i_172 = -1 + i_172) {volatile int keep = 1; if (keep) break;}

    for (int f_34 = 0; 10 > f_34; f_34 = -1 + f_34) {volatile int keep = 1; if (keep) break;}

    for (unsigned int i_173 = 0; i_173 < 10; i_173++) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_174 = 0; i_174 <= 10; i_174++) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_175 = 0; 10 > i_175; i_175++) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_176 = 0; 10 >= i_176; i_176++) {volatile int keep = 1; if (keep) break;}

    for (unsigned int f_35 = 0; 10 <= f_35; f_35++) {volatile int keep = 1; if (keep) break;}

    for (unsigned int i_177 = 0; i_177 > 10; i_177--) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_178 = 0; i_178 >= 10; i_178--) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_179 = 0; 10 < i_179 ; i_179--) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_180 = 0; 10 <= i_180; i_180--) {volatile int keep = 1; if (keep) break;}

    for (unsigned int f_36 = 0; 10 >= f_36; f_36--) {volatile int keep = 1; if (keep) break;}

    for (unsigned int i_181 = 0; i_181 < 10; i_181 += 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_182 = 0; i_182 <= 10; i_182 += 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_183 = 0; 10 > i_183; i_183 += 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_184 = 0; 10 >= i_184; i_184 += 1) {volatile int keep = 1; if (keep) break;}

    for (unsigned int f_37 = 0; 10 < f_37; f_37 += 1) {volatile int keep = 1; if (keep) break;}

    for (unsigned int i_185 = 0; i_185 > 10; i_185 -= 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_186 = 0; i_186 >= 10; i_186 -= 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_187 = 0; 10 < i_187; i_187 -= 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_188 = 0; 10 <= i_188; i_188 -= 1) {volatile int keep = 1; if (keep) break;}

    for (unsigned int f_38 = 0; 10 > f_38; f_38 -= 1) {volatile int keep = 1; if (keep) break;}

    for (unsigned int i_189 = 0; i_189 < 10; i_189 = i_189 + 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_190 = 0; i_190 <= 10; i_190 = i_190 + 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_191 = 0; 10 > i_191; i_191 = i_191 + 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_192 = 0; 10 >= i_192; i_192 = i_192 + 1) {volatile int keep = 1; if (keep) break;}

    for (unsigned int f_39 = 0; 10 <= f_39; f_39 = f_39 + 1) {volatile int keep = 1; if (keep) break;}

    for (unsigned int i_193 = 0; i_193 > 10; i_193 = i_193 - 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_194 = 0; i_194 >= 10; i_194 = i_194 - 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_195 = 0; 10 < i_195; i_195 = i_195 - 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_196 = 0; 10 <= i_196; i_196 = i_196 - 1) {volatile int keep = 1; if (keep) break;}

    for (unsigned int f_40 = 0; 10 >= f_40; f_40 = f_40 - 1) {volatile int keep = 1; if (keep) break;}

    for (unsigned int i_197 = 0; i_197 < 10; i_197 = 1 + i_197) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_198 = 0; i_198 <= 10; i_198 = 1 + i_198) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_199 = 0; 10 > i_199; i_199 = 1 + i_199) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_200 = 0; 10 >= i_200; i_200 = 1 + i_200) {volatile int keep = 1; if (keep) break;}

    for (unsigned int f_41 = 0; 10 < f_41; f_41 = 1 + f_41) {volatile int keep = 1; if (keep) break;}

    _signed_char = 1;
    for (int i_201 = 0; i_201 < 10; i_201 += _signed_char) {volatile int keep = 1; if (keep) break;}
    for (int i_202 = 0; i_202 <= 10; i_202 += _signed_char) {volatile int keep = 1; if (keep) break;}
    for (int i_203 = 0; 10 > i_203; i_203 += _signed_char) {volatile int keep = 1; if (keep) break;}
    for (int i_204 = 0; 10 >= i_204; i_204 += _signed_char) {volatile int keep = 1; if (keep) break;}

    for (int f_42 = 0; 10 < f_42; f_42 += _signed_char) {volatile int keep = 1; if (keep) break;}

    for (int i_205 = 0; i_205 > 10; i_205 -= _signed_char) {volatile int keep = 1; if (keep) break;}
    for (int i_206 = 0; i_206 >= 10; i_206 -= _signed_char) {volatile int keep = 1; if (keep) break;}
    for (int i_207 = 0; 10 < i_207; i_207 -= _signed_char) {volatile int keep = 1; if (keep) break;}
    for (int i_208 = 0; 10 <= i_208; i_208 -= _signed_char) {volatile int keep = 1; if (keep) break;}

    for (int f_43 = 0; 10 >= f_43; f_43 -= _signed_char) {volatile int keep = 1; if (keep) break;}

    _short = -1;
    for (int i_209 = 0; i_209 > 10; i_209 += _short) {volatile int keep = 1; if (keep) break;}
    for (int i_210 = 0; i_210 >= 10; i_210 += _short) {volatile int keep = 1; if (keep) break;}
    for (int i_211 = 0; 10 < i_211; i_211 += _short) {volatile int keep = 1; if (keep) break;}
    for (int i_212 = 0; 10 <= i_212; i_212 += _short) {volatile int keep = 1; if (keep) break;}

    for (int f_44 = 0; 10 >= f_44; f_44 += _short) {volatile int keep = 1; if (keep) break;}

    for (int i_213 = 0; i_213 < 10; i_213 -= _short) {volatile int keep = 1; if (keep) break;}
    for (int i_214 = 0; i_214 <= 10; i_214 -= _short) {volatile int keep = 1; if (keep) break;}
    for (int i_215 = 0; 10 > i_215; i_215 -= _short) {volatile int keep = 1; if (keep) break;}
    for (int i_216 = 0; 10 >= i_216; i_216 -= _short) {volatile int keep = 1; if (keep) break;}

    for (int f_45 = 0; 10 < f_45; f_45 -= _short) {volatile int keep = 1; if (keep) break;}

    stable_variable = 1;
    for (int i_217 = 0; i_217 < 10; i_217 = i_217 + stable_variable) {volatile int keep = 1; if (keep) break;}
    for (int i_218 = 0; i_218 <= 10; i_218 = i_218 + stable_variable) {volatile int keep = 1; if (keep) break;}
    for (int i_219 = 0; 10 > i_219; i_219 = i_219 + stable_variable) {volatile int keep = 1; if (keep) break;}
    for (int i_220 = 0; 10 >= i_220; i_220 = i_220 + stable_variable) {volatile int keep = 1; if (keep) break;}

    for (int f_46 = 0; 10 < f_46; f_46 = f_46 + stable_variable) {volatile int keep = 1; if (keep) break;}
    variable = 1;
    for (int f_47 = 0; 10 >= f_47; f_47 = f_47 + variable) {volatile int keep = 1; if (keep) break;}

    for (int i_221 = 0; i_221 > 10; i_221 = i_221 - stable_variable) {volatile int keep = 1; if (keep) break;}
    for (int i_222 = 0; i_222 >= 10; i_222 = i_222 - stable_variable) {volatile int keep = 1; if (keep) break;}
    for (int i_223 = 0; 10 < i_223; i_223 = i_223 - stable_variable) {volatile int keep = 1; if (keep) break;}
    for (int i_224 = 0; 10 <= i_224; i_224 = i_224 - stable_variable) {volatile int keep = 1; if (keep) break;}

    for (int f_48 = 0; 10 >= f_48; f_48 = f_48 - stable_variable) {volatile int keep = 1; if (keep) break;}

    _int8_t = -1;
    for (int i_225 = 0; i_225 > 10; i_225 = i_225 + _int8_t) {volatile int keep = 1; if (keep) break;}
    for (int i_226 = 0; i_226 >= 10; i_226 = i_226 + _int8_t) {volatile int keep = 1; if (keep) break;}
    for (int i_227 = 0; 10 < i_227; i_227 = i_227 + _int8_t) {volatile int keep = 1; if (keep) break;}
    for (int i_228 = 0; 10 <= i_228; i_228 = i_228 + _int8_t) {volatile int keep = 1; if (keep) break;}

    for (int f_49 = 0; 10 >= f_49; f_49 = f_49 + _int8_t) {volatile int keep = 1; if (keep) break;}

    for (int i_229 = 0; i_229 < 10; i_229 = i_229 - _int8_t) {volatile int keep = 1; if (keep) break;}
    for (int i_230 = 0; i_230 <= 10; i_230 = i_230 - _int8_t) {volatile int keep = 1; if (keep) break;}
    for (int i_231 = 0; 10 > i_231; i_231 = i_231 - _int8_t) {volatile int keep = 1; if (keep) break;}
    for (int i_232 = 0; 10 >= i_232; i_232 = i_232 - _int8_t) {volatile int keep = 1; if (keep) break;}

    for (int f_50 = 0; 10 < f_50; f_50 = f_50 - _int8_t) {volatile int keep = 1; if (keep) break;}

    _int16_t = 1;
    for (int i_233 = 0; i_233 < 10; i_233 = _int16_t + i_233) {volatile int keep = 1; if (keep) break;}
    for (int i_234 = 0; i_234 <= 10; i_234 = _int16_t + i_234) {volatile int keep = 1; if (keep) break;}
    for (int i_235 = 0; 10 > i_235; i_235 = _int16_t + i_235) {volatile int keep = 1; if (keep) break;}
    for (int i_236 = 0; 10 >= i_236; i_236 = _int16_t + i_236) {volatile int keep = 1; if (keep) break;}

    for (int f_51 = 0; 10 < f_51; f_51 = _int16_t + f_51) {volatile int keep = 1; if (keep) break;}

    _int16_t = -1;
    for (int i_237 = 0; i_237 > 10; i_237 = _int16_t + i_237) {volatile int keep = 1; if (keep) break;}
    for (int i_238 = 0; i_238 >= 10; i_238 = _int16_t + i_238) {volatile int keep = 1; if (keep) break;}
    for (int i_239 = 0; 10 < i_239; i_239 = _int16_t + i_239) {volatile int keep = 1; if (keep) break;}
    for (int i_240 = 0; 10 <= i_240; i_240 = _int16_t + i_240) {volatile int keep = 1; if (keep) break;}

    for (int f_52 = 0; 10 >= f_52; f_52 = _int16_t + f_52) {volatile int keep = 1; if (keep) break;}

    // body

    for (int f_53 = 0; f_53 < 10; f_53++) {f_53++; volatile int keep = 1; if (keep) break;}

    // END

    printf("End of test_idiomatize_loops().\n");
}

static void receive_t_values_for_l
(
    int variable,
    const int variable_const,
    int* pointer,
    const int* pointer_const_0,
    int* const pointer_const_1,
    outer_struct_t outer_struct,
    const outer_struct_t outer_struct_const,
    outer_struct_t* outer_struct_ptr,
    const outer_struct_t* outer_struct_ptr_const_0,
    outer_struct_t* const outer_struct_ptr_const_1
)
{
    for (int i_241 = variable; i_241 < 10; i_241++) {volatile int keep = 1; if (keep) break;}
    for (int i_242 = variable_const; i_242 < 10; i_242++) {volatile int keep = 1; if (keep) break;}
    for (int i_243 = *pointer; i_243 < 10; i_243++) {volatile int keep = 1; if (keep) break;}
    for (int i_244 = *pointer_const_0; i_244 < 10; i_244++) {volatile int keep = 1; if (keep) break;}
    for (int i_245 = *pointer_const_1; i_245 < 10; i_245++) {volatile int keep = 1; if (keep) break;}
    for (int i_246 = outer_struct.x; i_246 < 10; i_246++) {volatile int keep = 1; if (keep) break;}
    for (int i_247 = outer_struct.inner_struct.x; i_247 < 10; i_247++) {volatile int keep = 1; if (keep) break;}
    for (int i_248 = outer_struct_const.x; i_248 < 10; i_248++) {volatile int keep = 1; if (keep) break;}
    for (int i_249 = outer_struct_const.inner_struct.x; i_249 < 10; i_249++) {volatile int keep = 1; if (keep) break;}
    for (int i_250 = outer_struct_ptr->x; i_250 < 10; i_250++) {volatile int keep = 1; if (keep) break;}
    for (int i_251 = outer_struct_ptr->inner_struct.x; i_251 < 10; i_251++) {volatile int keep = 1; if (keep) break;}
    for (int i_252 = outer_struct_ptr_const_0->x; i_252 < 10; i_252++) {volatile int keep = 1; if (keep) break;}
    for (int i_253 = outer_struct_ptr_const_0->inner_struct.x; i_253 < 10; i_253++) {volatile int keep = 1; if (keep) break;}
    for (int i_254 = outer_struct_ptr_const_1->x; i_254 < 10; i_254++) {volatile int keep = 1; if (keep) break;}
    for (int i_255 = outer_struct_ptr_const_1->inner_struct.x; i_255 < 10; i_255++) {volatile int keep = 1; if (keep) break;}
}

static void receive_stable_t_values_for_n
(
    int stable_variable,
    const int stable_variable_const,
    outer_struct_t stable_outer_struct,
    const outer_struct_t stable_outer_struct_const
)
{
    for (int i_256 = 0; i_256 < stable_variable; i_256++) {volatile int keep = 1; if (keep) break;}
    for (int i_257 = 0; i_257 < stable_variable_const; i_257++) {volatile int keep = 1; if (keep) break;}
    for (int i_258 = 0; i_258 < stable_outer_struct.x; i_258++) {volatile int keep = 1; if (keep) break;}
    for (int i_259 = 0; i_259 < stable_outer_struct.inner_struct.x; i_259++) {volatile int keep = 1; if (keep) break;}
    for (int i_260 = 0; i_260 < stable_outer_struct_const.x; i_260++) {volatile int keep = 1; if (keep) break;}
    for (int i_261 = 0; i_261 < stable_outer_struct_const.inner_struct.x; i_261++) {volatile int keep = 1; if (keep) break;}

    int* stable_variable_ptr = &stable_variable;
    for (int f_54 = 0; f_54 < stable_variable; f_54++) {volatile int keep = 1; if (keep) break;}
    outer_struct_t* stable_outer_struct_ptr = &stable_outer_struct;
    for (int f_55 = 0; f_55 < stable_outer_struct.x; f_55++) {volatile int keep = 1; if (keep) break;}
}

static void receive_statically_determinable_t_values_for_n
(
    int stable_variable,
    outer_struct_t stable_outer_struct
)
{
    for (int f_56 = 0; f_56 > stable_variable; f_56--) {volatile int keep = 1; if (keep) break;}
    for (int f_57 = 0; stable_outer_struct.x < f_57; f_57--) {volatile int keep = 1; if (keep) break;}
    for (int f_58 = 0; f_58 > stable_outer_struct.inner_struct.x; f_58--) {volatile int keep = 1; if (keep) break;}

    stable_variable = 0;
    stable_outer_struct.x = 0;
    stable_outer_struct.inner_struct.x = 0;
    for (int i_262 = 0; i_262  > stable_variable; i_262 --) {volatile int keep = 1; if (keep) break;}
    for (int i_263 = 0; stable_outer_struct.x < i_263; i_263 --) {volatile int keep = 1; if (keep) break;}
    for (int i_264 = 0; i_264 > stable_outer_struct.inner_struct.x; i_264 --) {volatile int keep = 1; if (keep) break;}
}
