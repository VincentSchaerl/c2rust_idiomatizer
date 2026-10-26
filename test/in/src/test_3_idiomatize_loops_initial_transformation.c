#include <stdint.h>
#include <stdio.h>

#include "test_3_idiomatize_loops_initial_transformation.h"
#include "nested_struct.h"
#include "typedefs.h"

static int return_int();
static unsigned char return_unsigned_char();
static long long return_long_long();
static int8_t return_int8_t();
static uint64_t return_uint64_t();
static intptr_t return_intptr_t();
static uintptr_t return_uintptr_t();
static size_t return_size_t();
static typedef_unsigned_char return_typedef_unsigned_char();
static typedef_size_t return_typedef_size_t();

static void receive_t_values_for_l
(
    int variable,
    const int variable_const,
    int* pointer,
    const int* pointer_const_0,
    int* const pointer_const_1,
    int** pointer_2d,
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
extern int** pointer_2d_global;

int array_global_1[10];
static int array_global_static[10] = {0};
extern int array_2d_global[2][2];

extern outer_struct_t outer_struct_global;
static outer_struct_t outer_struct_global_static = { 0, { 0 } };

extern outer_struct_t* outer_struct_ptr_global;
extern const outer_struct_t* outer_struct_ptr_global_const_0;
static outer_struct_t* outer_struct_ptr_global_static = &outer_struct_global;

enum { type_alias_value = 0 };  // also stable

void test_3_idiomatize_loops_initial_transformation()
{
    // T-values

    int variable = 0;
    const int variable_const = 0;
    static int variable_static = 0;

    int* pointer = &variable;
    const int* pointer_const_0 = &variable_const;
    int* const pointer_const_1 = &variable;
    static int* pointer_static = &variable_static;
    int** pointer_2d = &pointer;

    int array[10] = {0};
    const int array_const[10] = {0};
    static int array_static[10] = {0};
    int array_2d[2][2] = {{0, 0}, {0, 0}};

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
    int stable_array_2d[2][2] = {{0, 0}, {0, 0}};

    outer_struct_t stable_outer_struct = { 0, { 0 } };
    const outer_struct_t stable_outer_struct_const = { 0, { 0 } };

    const int* stable_variable_const_ptr = &stable_variable_const;
    const int* stable_array_const_ptr_0 = stable_array_const;
    const int* stable_array_const_ptr_1 = &stable_array_const[1];
    const int (*stable_array_const_ptr_2)[10] = &stable_array_const;

    // (stable) U-values

    signed char _signed_char = 0;
    unsigned char _unsigned_char = 0;
    short _short = 0;
    unsigned short _unsigned_short = 0;
    int _int = 0;
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
    typedef_signed_char _typedef_signed_char = 0;
    typedef_unsigned_char _typedef_unsigned_char = 0;
    typedef_short _typedef_short = 0;
    typedef_unsigned_short _typedef_unsigned_short = 0;
    typedef_int _typedef_int = 0;
    typedef_unsigned_int _typedef_unsigned_int = 0;
    typedef_long _typedef_long = 0;
    typedef_unsigned_long _typedef_unsigned_long = 0;
    typedef_long_long _typedef_long_long = 0;
    typedef_unsigned_long_long _typedef_unsigned_long_long = 0;
    typedef_int8_t _typedef_int8_t = 0;
    typedef_uint8_t _typedef_uint8_t = 0;
    typedef_int16_t _typedef_int16_t = 0;
    typedef_uint16_t _typedef_uint16_t = 0;
    typedef_uint32_t _typedef_uint32_t = 0;
    typedef_int64_t _typedef_int64_t = 0;
    typedef_uint64_t _typedef_uint64_t = 0;
    typedef_intptr_t _typedef_intptr_t = 0;
    typedef_uintptr_t _typedef_uintptr_t = 0;
    typedef_size_t _typedef_size_t = 0;

    // basic form

    for (int i_0 = 0; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}

    for (int f_0 = 0; f_0 < 10; f_0++) {f_0++; volatile int keep = 1; if (keep) break;}
    for (int f_1 = 0; f_1 < 10; f_1++) {int* ptr = &f_1; volatile int keep = 1; if (keep) break;}

    // assignment_or_initialization / T

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
    for (intptr_t i_11 = 0; i_11 < 10; i_11++) {volatile int keep = 1; if (keep) break;}
    for (uintptr_t i_12 = 0; i_12 < 10; i_12++) {volatile int keep = 1; if (keep) break;}
    for (size_t i_13 = 0; i_13 < 10; i_13++) {volatile int keep = 1; if (keep) break;}
    for (typedef_int i_14 = 0; i_14 < 10; i_14++) {volatile int keep = 1; if (keep) break;}
    for (typedef_unsigned_int i_15 = 0; i_15 < 10; i_15++) {volatile int keep = 1; if (keep) break;}
    for (typedef_long i_16 = 0; i_16 < 10; i_16++) {volatile int keep = 1; if (keep) break;}
    for (typedef_unsigned_long i_17 = 0; i_17 < 10; i_17++) {volatile int keep = 1; if (keep) break;}
    for (typedef_long_long i_18 = 0; i_18 < 10; i_18++) {volatile int keep = 1; if (keep) break;}
    for (typedef_unsigned_long_long i_19 = 0; i_19 < 10; i_19++) {volatile int keep = 1; if (keep) break;}
    for (typedef_int32_t i_20 = 0; i_20 < 10; i_20++) {volatile int keep = 1; if (keep) break;}
    for (typedef_uint32_t i_21 = 0; i_21 < 10; i_21++) {volatile int keep = 1; if (keep) break;}
    for (typedef_int64_t i_22 = 0; i_22 < 10; i_22++) {volatile int keep = 1; if (keep) break;}
    for (typedef_uint64_t i_23 = 0; i_23 < 10; i_23++) {volatile int keep = 1; if (keep) break;}
    for (typedef_intptr_t i_24 = 0; i_24 < 10; i_24++) {volatile int keep = 1; if (keep) break;}
    for (typedef_uintptr_t i_25 = 0; i_25 < 10; i_25++) {volatile int keep = 1; if (keep) break;}
    for (typedef_size_t i_26 = 0; i_26 < 10; i_26++) {volatile int keep = 1; if (keep) break;}

    int i_27 = 0;
    for (i_27 = 0; i_27 < 10; i_27++) {volatile int keep = 1; if (keep) break;}
    i_27 = 1;
    unsigned int i_28 = 0;
    for (i_28 = 0; i_28 < 10; i_28++) {volatile int keep = 1; if (keep) break;}
    i_28 = 1;
    long i_29 = 0;
    for (i_29 = 0; i_29 < 10; i_29++) {volatile int keep = 1; if (keep) break;}
    i_29 = 1;
    unsigned long i_30 = 0;
    for (i_30 = 0; i_30 < 10; i_30++) {volatile int keep = 1; if (keep) break;}
    i_30 = 1;
    long long i_31 = 0;
    for (i_31 = 0; i_31 < 10; i_31++) {volatile int keep = 1; if (keep) break;}
    i_31 = 1;
    unsigned long long i_32 = 0;
    for (i_32 = 0; i_32 < 10; i_32++) {volatile int keep = 1; if (keep) break;}
    i_32 = 1;
    int32_t i_33 = 0;
    for (i_33 = 0; i_33 < 10; i_33++) {volatile int keep = 1; if (keep) break;}
    i_33 = 1;
    uint32_t i_34 = 0;
    for (i_34 = 0; i_34 < 10; i_34++) {volatile int keep = 1; if (keep) break;}
    i_34 = 1;
    int64_t i_35 = 0;
    for (i_35 = 0; i_35 < 10; i_35++) {volatile int keep = 1; if (keep) break;}
    i_35 = 1;
    uint64_t i_36 = 0;
    for (i_36 = 0; i_36 < 10; i_36++) {volatile int keep = 1; if (keep) break;}
    i_36 = 1;
    intptr_t i_37 = 0;
    for (i_37 = 0; i_37 < 10; i_37++) {volatile int keep = 1; if (keep) break;}
    i_37 = 1;
    uintptr_t i_38 = 0;
    for (i_38 = 0; i_38 < 10; i_38++) {volatile int keep = 1; if (keep) break;}
    i_38 = 1;
    size_t i_39 = 0;
    for (i_39 = 0; i_39 < 10; i_39++) {volatile int keep = 1; if (keep) break;}
    i_39 = 1;
    typedef_int i_40 = 0;
    for (i_40 = 0; i_40 < 10; i_40++) {volatile int keep = 1; if (keep) break;}
    i_40 = 1;
    typedef_unsigned_int i_41 = 0;
    for (i_41 = 0; i_41 < 10; i_41++) {volatile int keep = 1; if (keep) break;}
    i_41 = 1;
    typedef_long i_42 = 0;
    for (i_42 = 0; i_42 < 10; i_42++) {volatile int keep = 1; if (keep) break;}
    i_42 = 1;
    typedef_unsigned_long i_43 = 0;
    for (i_43 = 0; i_43 < 10; i_43++) {volatile int keep = 1; if (keep) break;}
    i_43 = 1;
    typedef_long_long i_44 = 0;
    for (i_44 = 0; i_44 < 10; i_44++) {volatile int keep = 1; if (keep) break;}
    i_44 = 1;
    typedef_unsigned_long_long i_45 = 0;
    for (i_45 = 0; i_45 < 10; i_45++) {volatile int keep = 1; if (keep) break;}
    i_45 = 1;
    typedef_int32_t i_46 = 0;
    for (i_46 = 0; i_46 < 10; i_46++) {volatile int keep = 1; if (keep) break;}
    i_46 = 1;
    typedef_uint32_t i_47 = 0;
    for (i_47 = 0; i_47 < 10; i_47++) {volatile int keep = 1; if (keep) break;}
    i_47 = 1;
    typedef_int64_t i_48 = 0;
    for (i_48 = 0; i_48 < 10; i_48++) {volatile int keep = 1; if (keep) break;}
    i_48 = 1;
    typedef_uint64_t i_49 = 0;
    for (i_49 = 0; i_49 < 10; i_49++) {volatile int keep = 1; if (keep) break;}
    i_49 = 1;
    typedef_intptr_t i_50 = 0;
    for (i_50 = 0; i_50 < 10; i_50++) {volatile int keep = 1; if (keep) break;}
    i_50 = 1;
    typedef_uintptr_t i_51 = 0;
    for (i_51 = 0; i_51 < 10; i_51++) {volatile int keep = 1; if (keep) break;}
    i_51 = 1;
    typedef_size_t i_52 = 0;
    for (i_52 = 0; i_52 < 10; i_52++) {volatile int keep = 1; if (keep) break;}
    i_52 = 1;

    for (char f_2 = 0; f_2 < 10; f_2++) {volatile int keep = 1; if (keep) break;}
    for (typedef_char f_3 = 0; f_3 < 10; f_3++) {volatile int keep = 1; if (keep) break;}
    static int f_4 = 0;
    for (f_4 = 0; f_4 < 10; f_4++) {volatile int keep = 1; if (keep) break;}
    int f_5 = 0;
    int* f_5_ptr = &f_5;
    for (f_5 = 0; f_5 < 10; f_5++) {volatile int keep = 1; if (keep) break;}
    int f_6 = 0;
    for (f_6 = 0; f_6 < 10; f_6++)
    {
        int* f_6_ptr = &f_6;
        volatile int keep = 1; if (keep) break;
    }
    int f_7 = 0;
    for (f_7 = 0; f_7 < 10; f_7++) {volatile int keep = 1; if (keep) break;}
    int x = f_7;

    // L

    for (int i_53 = 0; i_53 < 10; i_53++) {volatile int keep = 1; if (keep) break;}
    for (int i_54 = -3; i_54 < 10; i_54++) {volatile int keep = 1; if (keep) break;}
    for (int i_55 = 6; i_55 < 10; i_55++) {volatile int keep = 1; if (keep) break;}
    for (int i_56 = -011; i_56 < 10; i_56++) {volatile int keep = 1; if (keep) break;}
    for (int i_57 = 0xC; i_57 < 10; i_57++) {volatile int keep = 1; if (keep) break;}

    for (int i_58 = variable_global; i_58 < 10; i_58++) {volatile int keep = 1; if (keep) break;}
    for (int i_59 = VARIABLE_GLOBAL_CONST; i_59 < 10; i_59++) {volatile int keep = 1; if (keep) break;}
    for (int i_60 = variable_global_static; i_60 < 10; i_60++) {volatile int keep = 1; if (keep) break;}
    for (int i_61 = variable; i_61 < 10; i_61++) {volatile int keep = 1; if (keep) break;}
    for (int i_62 = variable_const; i_62 < 10; i_62++) {volatile int keep = 1; if (keep) break;}
    for (int i_63 = variable_static; i_63 < 10; i_63++) {volatile int keep = 1; if (keep) break;}

    for (int i_64 = *pointer_global; i_64 < 10; i_64++) {volatile int keep = 1; if (keep) break;}
    for (int i_65 = *pointer_global_const_0; i_65 < 10; i_65++) {volatile int keep = 1; if (keep) break;}
    for (int i_66 = *pointer_global_static; i_66 < 10; i_66++) {volatile int keep = 1; if (keep) break;}
    for (int i_67 = **pointer_2d_global; i_67 < 10; i_67++) {volatile int keep = 1; if (keep) break;}
    for (int i_68 = *pointer; i_68 < 10; i_68++) {volatile int keep = 1; if (keep) break;}
    for (int i_69 = *pointer_const_0; i_69 < 10; i_69++) {volatile int keep = 1; if (keep) break;}
    for (int i_70 = *pointer_const_1; i_70 < 10; i_70++) {volatile int keep = 1; if (keep) break;}
    for (int i_71 = *pointer_static; i_71 < 10; i_71++) {volatile int keep = 1; if (keep) break;}
    for (int i_72 = **pointer_2d; i_72 < 10; i_72++) {volatile int keep = 1; if (keep) break;}

    const int i = 0;
    for (int i_73 = array_global_1[0]; i_73 < 10; i_73++) {volatile int keep = 1; if (keep) break;}
    for (int i_74 = array_global_1[i]; i_74 < 10; i_74++) {volatile int keep = 1; if (keep) break;}
    for (int i_75 = array_global_static[0]; i_75 < 10; i_75++) {volatile int keep = 1; if (keep) break;}
    for (int i_76 = array_global_static[i]; i_76 < 10; i_76++) {volatile int keep = 1; if (keep) break;}
    for (int i_77 = array_2d_global[0][0]; i_77 < 10; i_77++) {volatile int keep = 1; if (keep) break;}
    for (int i_78 = array_2d_global[i][i]; i_78 < 10; i_78++) {volatile int keep = 1; if (keep) break;}
    for (int i_79 = array[0]; i_79 < 10; i_79++) {volatile int keep = 1; if (keep) break;}
    for (int i_80 = array[i]; i_80 < 10; i_80++) {volatile int keep = 1; if (keep) break;}
    for (int i_81 = array_const[0]; i_81 < 10; i_81++) {volatile int keep = 1; if (keep) break;}
    for (int i_82 = array_const[i]; i_82 < 10; i_82++) {volatile int keep = 1; if (keep) break;}
    for (int i_83 = array_static[0]; i_83 < 10; i_83++) {volatile int keep = 1; if (keep) break;}
    for (int i_84 = array_static[i]; i_84 < 10; i_84++) {volatile int keep = 1; if (keep) break;}
    for (int i_85 = array_2d[0][0]; i_85 < 10; i_85++) {volatile int keep = 1; if (keep) break;}
    for (int i_86 = array_2d[i][i]; i_86 < 10; i_86++) {volatile int keep = 1; if (keep) break;}

    for (int i_87 = outer_struct_global.x; i_87 < 10; i_87++) {volatile int keep = 1; if (keep) break;}
    for (int i_88 = outer_struct_global.inner_struct.x; i_88 < 10; i_88++) {volatile int keep = 1; if (keep) break;}
    for (int i_89 = outer_struct_global_static.x; i_89 < 10; i_89++) {volatile int keep = 1; if (keep) break;}
    for (int i_90 = outer_struct_global_static.inner_struct.x; i_90 < 10; i_90++)
        {volatile int keep = 1; if (keep) break;}
    for (int i_91 = outer_struct.x; i_91 < 10; i_91++) {volatile int keep = 1; if (keep) break;}
    for (int i_92 = outer_struct.inner_struct.x; i_92 < 10; i_92++) {volatile int keep = 1; if (keep) break;}
    for (int i_93 = outer_struct_const.x; i_93 < 10; i_93++) {volatile int keep = 1; if (keep) break;}
    for (int i_94 = outer_struct_const.inner_struct.x; i_94 < 10; i_94++) {volatile int keep = 1; if (keep) break;}
    for (int i_95 = outer_struct_static.x; i_95 < 10; i_95++) {volatile int keep = 1; if (keep) break;}
    for (int i_96 = outer_struct_static.inner_struct.x; i_96 < 10; i_96++) {volatile int keep = 1; if (keep) break;}

    for (int i_97 = outer_struct_ptr_global->x; i_97 < 10; i_97++) {volatile int keep = 1; if (keep) break;}
    for (int i_98 = outer_struct_ptr_global->inner_struct.x; i_98 < 10; i_98++)
        {volatile int keep = 1; if (keep) break;}
    for (int i_99 = outer_struct_ptr_global_const_0->x; i_99 < 10; i_99++) {volatile int keep = 1; if (keep) break;}
    for (int i_100 = outer_struct_ptr_global_const_0->inner_struct.x; i_100 < 10; i_100++)
        {volatile int keep = 1; if (keep) break;}
    for (int i_101 = outer_struct_ptr_global_static->x; i_101 < 10; i_101++)
        {volatile int keep = 1; if (keep) break;}
    for (int i_102 = outer_struct_ptr_global_static->inner_struct.x; i_102 < 10; i_102++)
        {volatile int keep = 1; if (keep) break;}
    for (int i_103 = outer_struct_ptr->x; i_103 < 10; i_103++) {volatile int keep = 1; if (keep) break;}
    for (int i_104 = outer_struct_ptr->inner_struct.x; i_104 < 10; i_104++) {volatile int keep = 1; if (keep) break;}
    for (int i_105 = outer_struct_ptr_const_0->x; i_105 < 10; i_105++) {volatile int keep = 1; if (keep) break;}
    for (int i_106 = outer_struct_ptr_const_0->inner_struct.x; i_106 < 10; i_106++)
        {volatile int keep = 1; if (keep) break;}
    for (int i_107 = outer_struct_ptr_const_1->x; i_107 < 10; i_107++) {volatile int keep = 1; if (keep) break;}
    for (int i_108 = outer_struct_ptr_const_1->inner_struct.x; i_108 < 10; i_108++)
        {volatile int keep = 1; if (keep) break;}
    for (int i_109 = outer_struct_ptr_static->x; i_109 < 10; i_109++) {volatile int keep = 1; if (keep) break;}
    for (int i_110 = outer_struct_ptr_static->inner_struct.x; i_110 < 10; i_110++)
        {volatile int keep = 1; if (keep) break;}
    for (int i_111 = return_int(); i_111 < 10; i_111++) {volatile int keep = 1; if (keep) break;}

    for (int i_112 = type_alias_value; i_112 < 10; i_112++) {volatile int keep = 1; if (keep) break;}

    receive_t_values_for_l
    (
        variable,
        variable_const,
        pointer,
        pointer_const_0,
        pointer_const_1,
        pointer_2d,
        outer_struct,
        outer_struct_const,
        outer_struct_ptr,
        outer_struct_ptr_const_0,
        outer_struct_ptr_const_1
    );

    for (int i_113 = _signed_char; i_113 < 10; i_113++) {volatile int keep = 1; if (keep) break;}
    for (int i_114 = _unsigned_char; i_114 < 10; i_114++) {volatile int keep = 1; if (keep) break;}
    for (int i_115 = _short; i_115 < 10; i_115++) {volatile int keep = 1; if (keep) break;}
    for (int i_116 = _unsigned_short; i_116 < 10; i_116++) {volatile int keep = 1; if (keep) break;}
    for (int i_117 = _int; i_117 < 10; i_117++) {volatile int keep = 1; if (keep) break;}
    for (int i_118 = _unsigned_int; i_118 < 10; i_118++) {volatile int keep = 1; if (keep) break;}
    for (int i_119 = _long; i_119 < 10; i_119++) {volatile int keep = 1; if (keep) break;}
    for (int i_120 = _unsigned_long; i_120 < 10; i_120++) {volatile int keep = 1; if (keep) break;}
    for (int i_121 = _long_long; i_121 < 10; i_121++) {volatile int keep = 1; if (keep) break;}
    for (int i_122 = _unsigned_long_long; i_122 < 10; i_122++) {volatile int keep = 1; if (keep) break;}
    for (int i_123 = _int8_t; i_123 < 10; i_123++) {volatile int keep = 1; if (keep) break;}
    for (int i_124 = _uint8_t; i_124 < 10; i_124++) {volatile int keep = 1; if (keep) break;}
    for (int i_125 = _int16_t; i_125 < 10; i_125++) {volatile int keep = 1; if (keep) break;}
    for (int i_126 = _uint16_t; i_126 < 10; i_126++) {volatile int keep = 1; if (keep) break;}
    for (int i_127 = _uint32_t; i_127 < 10; i_127++) {volatile int keep = 1; if (keep) break;}
    for (int i_128 = _int64_t; i_128 < 10; i_128++) {volatile int keep = 1; if (keep) break;}
    for (int i_129 = _uint64_t; i_129 < 10; i_129++) {volatile int keep = 1; if (keep) break;}
    for (int i_130 = _intptr_t; i_130 < 10; i_130++) {volatile int keep = 1; if (keep) break;}
    for (int i_131 = _uintptr_t; i_131 < 10; i_131++) {volatile int keep = 1; if (keep) break;}
    for (int i_132 = _size_t; i_132 < 10; i_132++) {volatile int keep = 1; if (keep) break;}
    for (int i_133 = _typedef_signed_char; i_133 < 10; i_133++) {volatile int keep = 1; if (keep) break;}
    for (int i_134 = _typedef_unsigned_char; i_134 < 10; i_134++) {volatile int keep = 1; if (keep) break;}
    for (int i_135 = _typedef_short; i_135 < 10; i_135++) {volatile int keep = 1; if (keep) break;}
    for (int i_136 = _typedef_unsigned_short; i_136 < 10; i_136++) {volatile int keep = 1; if (keep) break;}
    for (int i_137 = _typedef_int; i_137 < 10; i_137++) {volatile int keep = 1; if (keep) break;}
    for (int i_138 = _typedef_unsigned_int; i_138 < 10; i_138++) {volatile int keep = 1; if (keep) break;}
    for (int i_139 = _typedef_long; i_139 < 10; i_139++) {volatile int keep = 1; if (keep) break;}
    for (int i_140 = _typedef_unsigned_long; i_140 < 10; i_140++) {volatile int keep = 1; if (keep) break;}
    for (int i_141 = _typedef_long_long; i_141 < 10; i_141++) {volatile int keep = 1; if (keep) break;}
    for (int i_142 = _typedef_unsigned_long_long; i_142 < 10; i_142++) {volatile int keep = 1; if (keep) break;}
    for (int i_143 = _typedef_int8_t; i_143 < 10; i_143++) {volatile int keep = 1; if (keep) break;}
    for (int i_144 = _typedef_uint8_t; i_144 < 10; i_144++) {volatile int keep = 1; if (keep) break;}
    for (int i_145 = _typedef_int16_t; i_145 < 10; i_145++) {volatile int keep = 1; if (keep) break;}
    for (int i_146 = _typedef_uint16_t; i_146 < 10; i_146++) {volatile int keep = 1; if (keep) break;}
    for (int i_147 = _typedef_uint32_t; i_147 < 10; i_147++) {volatile int keep = 1; if (keep) break;}
    for (int i_148 = _typedef_int64_t; i_148 < 10; i_148++) {volatile int keep = 1; if (keep) break;}
    for (int i_149 = _typedef_uint64_t; i_149 < 10; i_149++) {volatile int keep = 1; if (keep) break;}
    for (int i_150 = _typedef_intptr_t; i_150 < 10; i_150++) {volatile int keep = 1; if (keep) break;}
    for (int i_151 = _typedef_uintptr_t; i_151 < 10; i_151++) {volatile int keep = 1; if (keep) break;}
    for (int i_152 = _typedef_size_t; i_152 < 10; i_152++) {volatile int keep = 1; if (keep) break;}

    for (int i_153 = return_unsigned_char(); i_153 < 10; i_153++)  {volatile int keep = 1; if (keep) break;}
    for (int i_154 = return_long_long(); i_154 < 10; i_154++)  {volatile int keep = 1; if (keep) break;}
    for (int i_155 = return_int8_t(); i_155 < 10; i_155++)  {volatile int keep = 1; if (keep) break;}
    for (int i_156 = return_uint64_t(); i_156 < 10; i_156++)  {volatile int keep = 1; if (keep) break;}
    for (int i_157 = return_intptr_t(); i_157 < 10; i_157++)  {volatile int keep = 1; if (keep) break;}
    for (int i_158 = return_uintptr_t(); i_158 < 10; i_158++)  {volatile int keep = 1; if (keep) break;}
    for (int i_159 = return_size_t(); i_159 < 10; i_159++)  {volatile int keep = 1; if (keep) break;}
    for (int i_160 = return_typedef_unsigned_char(); i_160 < 10; i_160++)  {volatile int keep = 1; if (keep) break;}
    for (int i_161 = return_typedef_size_t(); i_161 < 10; i_161++)  {volatile int keep = 1; if (keep) break;}

    // relational_operator

    for (int i_162 = 0; i_162 < 10; i_162++) {volatile int keep = 1; if (keep) break;}
    for (int i_163 = 0; i_163 <= 10; i_163++) {volatile int keep = 1; if (keep) break;}
    for (int i_164 = 0; i_164 > 10; i_164--) {volatile int keep = 1; if (keep) break;}
    for (int i_165 = 0; i_165 >= 10; i_165--) {volatile int keep = 1; if (keep) break;}
    for (int i_166 = 0; 10 < i_166; i_166--) {volatile int keep = 1; if (keep) break;}
    for (int i_167 = 0; 10 <= i_167; i_167--) {volatile int keep = 1; if (keep) break;}
    for (int i_168 = 0; 10 > i_168; i_168++) {volatile int keep = 1; if (keep) break;}
    for (int i_169 = 0; 10 >= i_169; i_169++) {volatile int keep = 1; if (keep) break;}

    for (int f_8 = 0; f_8 != 10; f_8++) {volatile int keep = 1; if (keep) break;}

    // N

    for (int i_170 = 0; i_170 < 0; i_170++) {volatile int keep = 1; if (keep) break;}
    for (int i_171 = 0; i_171 < -3; i_171++) {volatile int keep = 1; if (keep) break;}
    for (int i_172 = 0; i_172 < 6; i_172++) {volatile int keep = 1; if (keep) break;}
    for (int i_173 = 0; i_173 < -011; i_173++) {volatile int keep = 1; if (keep) break;}
    for (int i_174 = 0; i_174 < 0xC; i_174++) {volatile int keep = 1; if (keep) break;}

    for (int i_175 = 0; i_175 < VARIABLE_GLOBAL_CONST; i_175++) {volatile int keep = 1; if (keep) break;}
    for (int i_176 = 0; i_176 < stable_variable; i_176++) {volatile int keep = 1; if (keep) break;}
    for (int i_177 = 0; i_177 < stable_variable_const; i_177++) {volatile int keep = 1; if (keep) break;}
    int stable_variable_temp_0 = 0;
    for (int i_178 = 0; i_178 < stable_variable_temp_0; i_178++) {volatile int keep = 1; if (keep) break;}
    int* stable_variable_temp_0_ptr = &stable_variable_temp_0;

    stable_variable = 0;
    _size_t = 0;
    stable_array[0] = 0;
    stable_outer_struct.x = 0;
    for (int i_179 = 0; i_179 < stable_array[0]; i_179++) {volatile int keep = 1; if (keep) break;}
    for (int i_180 = 0; i_180 < stable_array[stable_variable]; i_180++) {volatile int keep = 1; if (keep) break;}
    for (int i_181 = 0; i_181 < stable_array[_size_t]; i_181++) {volatile int keep = 1; if (keep) break;}
    for (int i_182 = 0; i_182 < stable_array[stable_array[0]]; i_182++) {volatile int keep = 1; if (keep) break;}
    for (int i_183 = 0; i_183 < stable_array[stable_array[stable_variable]]; i_183++)
    {
        volatile int keep = 1;
        if (keep) break;
    }
    for (int i_184 = 0; i_184 < stable_array[stable_array[_size_t]]; i_184++)
        {volatile int keep = 1; if (keep) break;}
    for (int i_185 = 0; i_185 < stable_array[stable_array[stable_array[0]]]; i_185++)
        {volatile int keep = 1; if (keep) break;}
    for (int i_186 = 0; i_186 < stable_array[stable_array[stable_outer_struct.x]]; i_186++)
        {volatile int keep = 1; if (keep) break;}
    for (int i_187 = 0; i_187 < stable_array[stable_outer_struct.x]; i_187++)
        {volatile int keep = 1; if (keep) break;}
    for (int i_188 = 0; i_188 < stable_array_const[0]; i_188++) {volatile int keep = 1; if (keep) break;}
    for (int i_189 = 0; i_189 < stable_array_2d[0][0]; i_189++) {volatile int keep = 1; if (keep) break;}
    for (int i_190 = 0; i_190 < stable_array_2d[stable_variable][stable_variable]; i_190++)
        {volatile int keep = 1; if (keep) break;}
    for (int i_191 = 0; i_191 < stable_array_2d[_size_t][_size_t]; i_191++) {volatile int keep = 1; if (keep) break;}
    for (int i_192 = 0; i_192 < stable_array_2d[stable_array[0]][stable_array[0]]; i_192++)
        {volatile int keep = 1; if (keep) break;}
    for (
        int i_193 = 0;
        i_193 < stable_array_2d[stable_array[stable_variable]][stable_array[stable_variable]];
        i_193++
    )
        {volatile int keep = 1; if (keep) break;}
    for (int i_194 = 0; i_194 < stable_array_2d[stable_array[_size_t]][stable_array[_size_t]]; i_194++)
        {volatile int keep = 1; if (keep) break;}
    for (
        int i_195 = 0;
        i_195 < stable_array_2d[stable_array[stable_array[0]]][stable_array[stable_array[0]]];
        i_195++
    )
        {volatile int keep = 1; if (keep) break;}
    for (
        int i_196 = 0;
        i_196 < stable_array_2d[stable_array[stable_outer_struct.x]][stable_array[stable_outer_struct.x]];
        i_196++
    )
        {volatile int keep = 1; if (keep) break;}
    for (int i_197 = 0; i_197 < stable_array_2d[stable_outer_struct.x][stable_outer_struct.x]; i_197++)
        {volatile int keep = 1; if (keep) break;}
    int stable_array_temp_0[10] = {0};
    for (int i_198 = 0; i_198 < stable_array_temp_0[0]; i_198++) {volatile int keep = 1; if (keep) break;}
    int* stable_array_temp_0_ptr_0 = stable_array_temp_0;
    int* stable_array_temp_0_ptr_1 = &stable_array_temp_0[1];
    int (*stable_array_temp_0_ptr_2)[10] = &stable_array_temp_0;

    for (int i_199 = 0; i_199 < stable_outer_struct.x; i_199++) {volatile int keep = 1; if (keep) break;}
    for (int i_200 = 0; i_200 < stable_outer_struct.inner_struct.x; i_200++) {volatile int keep = 1; if (keep) break;}
    for (int i_201 = 0; i_201 < stable_outer_struct_const.x; i_201++) {volatile int keep = 1; if (keep) break;}
    for (int i_202 = 0; i_202 < stable_outer_struct_const.inner_struct.x; i_202++)
        {volatile int keep = 1; if (keep) break;}
    outer_struct_t stable_outer_struct_temp_0 = { 0, { 0 } };
    for (int i_203 = 0; i_203 < stable_outer_struct_temp_0.x; i_203++) {volatile int keep = 1; if (keep) break;}
    for (int i_204 = 0; i_204 < stable_outer_struct_temp_0.inner_struct.x; i_204++)
        {volatile int keep = 1; if (keep) break;}
    int* stable_outer_struct_temp_0_ptr_0 = &stable_outer_struct_temp_0.inner_struct.x;
    outer_struct_t* stable_outer_struct_temp_0_ptr_1 = &stable_outer_struct_temp_0;

    for (int i_205 = 0; i_205 < type_alias_value; i_205++) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_206 = 0; i_206 < type_alias_value; i_206++) {volatile int keep = 1; if (keep) break;}
    for (long long i_207 = 0; i_207 < type_alias_value; i_207++) {volatile int keep = 1; if (keep) break;}
    for (unsigned long long i_208 = 0; i_208 < type_alias_value; i_208++) {volatile int keep = 1; if (keep) break;}
    for (int32_t i_209 = 0; i_209 < type_alias_value; i_209++) {volatile int keep = 1; if (keep) break;}
    for (uint32_t i_210 = 0; i_210 < type_alias_value; i_210++) {volatile int keep = 1; if (keep) break;}
    for (int64_t i_211 = 0; i_211 < type_alias_value; i_211++) {volatile int keep = 1; if (keep) break;}
    for (uint64_t i_212 = 0; i_212 < type_alias_value; i_212++) {volatile int keep = 1; if (keep) break;}
    for (intptr_t i_213 = 0; i_213 < type_alias_value; i_213++) {volatile int keep = 1; if (keep) break;}
    for (uintptr_t i_214 = 0; i_214 < type_alias_value; i_214++) {volatile int keep = 1; if (keep) break;}
    for (size_t i_215 = 0; i_215 < type_alias_value; i_215++) {volatile int keep = 1; if (keep) break;}

    for (int32_t i_216 = 0; i_216 < INT16_MAX; i_216++) {volatile int keep = 1; if (keep) break;}
    for (int32_t i_217 = 0; INT16_MAX > i_217; i_217++) {volatile int keep = 1; if (keep) break;}

    receive_stable_t_values_for_n
    (
        stable_variable,
        stable_variable_const,
        stable_outer_struct,
        stable_outer_struct_const
    );

    for (int i_218 = 0; i_218 < _signed_char; i_218++) {volatile int keep = 1; if (keep) break;}
    for (int i_219 = 0; i_219 < _unsigned_char; i_219++) {volatile int keep = 1; if (keep) break;}
    for (int i_220 = 0; i_220 < _short; i_220++) {volatile int keep = 1; if (keep) break;}
    for (int i_221 = 0; i_221 < _unsigned_short; i_221++) {volatile int keep = 1; if (keep) break;}
    for (int i_222 = 0; i_222 < _int; i_222++) {volatile int keep = 1; if (keep) break;}
    for (int i_223 = 0; i_223 < _int8_t; i_223++) {volatile int keep = 1; if (keep) break;}
    for (int i_224 = 0; i_224 < _uint8_t; i_224++) {volatile int keep = 1; if (keep) break;}
    for (int i_225 = 0; i_225 < _int16_t; i_225++) {volatile int keep = 1; if (keep) break;}
    for (int i_226 = 0; i_226 < _uint16_t; i_226++) {volatile int keep = 1; if (keep) break;}
    for (int i_227 = 0; i_227 < _typedef_signed_char; i_227++) {volatile int keep = 1; if (keep) break;}
    for (int i_228 = 0; i_228 < _typedef_unsigned_char; i_228++) {volatile int keep = 1; if (keep) break;}
    for (int i_229 = 0; i_229 < _typedef_short; i_229++) {volatile int keep = 1; if (keep) break;}
    for (int i_230 = 0; i_230 < _typedef_unsigned_short; i_230++) {volatile int keep = 1; if (keep) break;}
    for (int i_231 = 0; i_231 < _int; i_231++) {volatile int keep = 1; if (keep) break;}
    for (int i_232 = 0; i_232 < _typedef_int8_t; i_232++) {volatile int keep = 1; if (keep) break;}
    for (int i_233 = 0; i_233 < _typedef_uint8_t; i_233++) {volatile int keep = 1; if (keep) break;}
    for (int i_234 = 0; i_234 < _typedef_int16_t; i_234++) {volatile int keep = 1; if (keep) break;}
    for (int i_235 = 0; i_235 < _typedef_uint16_t; i_235++) {volatile int keep = 1; if (keep) break;}

    stable_array[0] = 0;
    for (int i_236 = 0; i_236 > 10; i_236--) {volatile int keep = 1; if (keep) break;}
    for (int i_237 = 0; VARIABLE_GLOBAL_CONST < i_237; i_237--) {volatile int keep = 1; if (keep) break;}
    for (int i_238 = 0; i_238 > stable_variable; i_238--) {volatile int keep = 1; if (keep) break;}
    for (int i_239 = 0; i_239 > stable_variable_const; i_239--) {volatile int keep = 1; if (keep) break;}
    for (int i_240 = 0; stable_array[0] < i_240; i_240--) {volatile int keep = 1; if (keep) break;}
    for (int i_241 = 0; stable_array_const[0] < i_241; i_241--) {volatile int keep = 1; if (keep) break;}
    for (int i_242 = 0; stable_array_2d[0][0] < i_242; i_242--) {volatile int keep = 1; if (keep) break;}
    for (int i_243 = 0; i_243 > stable_outer_struct.x; i_243--) {volatile int keep = 1; if (keep) break;}
    for (int i_244 = 0; i_244 > stable_outer_struct_const.inner_struct.x; i_244--)
        {volatile int keep = 1; if (keep) break;}
    for (int i_245 = 0; type_alias_value < i_245; i_245--) {volatile int keep = 1; if (keep) break;}
    for (int i_246 = 0; _signed_char < i_246; i_246--) {volatile int keep = 1; if (keep) break;}
    for (int i_247 = 0; _unsigned_char < i_247; i_247--) {volatile int keep = 1; if (keep) break;}
    for (int i_248 = 0; _short < i_248; i_248--) {volatile int keep = 1; if (keep) break;}
    for (int i_249 = 0; _unsigned_short < i_249; i_249--) {volatile int keep = 1; if (keep) break;}
    for (int i_250 = 0; _int8_t < i_250; i_250--) {volatile int keep = 1; if (keep) break;}
    for (int i_251 = 0; _uint8_t < i_251; i_251--) {volatile int keep = 1; if (keep) break;}
    for (int i_252 = 0; _int16_t < i_252; i_252--) {volatile int keep = 1; if (keep) break;}
    for (int i_253 = 0; _uint16_t < i_253; i_253--) {volatile int keep = 1; if (keep) break;}

    receive_statically_determinable_t_values_for_n
    (
        stable_variable,
        stable_outer_struct
    );

    for (int f_9 = 0; f_9 < variable; f_9++) {volatile int keep = 1; if (keep) break;}
    static int variable_static_temp = 0;
    for (int f_10 = 0; f_10 < variable_static_temp; f_10++) {volatile int keep = 1; if (keep) break;}
    int variable_temp_0 = 0;
    for (int f_11 = 0; f_11 < variable_temp_0; f_11++)
    {
        int* variable_temp_0_ptr = &variable_temp_0;
        volatile int keep = 1;
        if (keep) break;
    }
    for (int f_12 = 0; f_12 < stable_variable; f_12++) {stable_variable = 0; volatile int keep = 1; if (keep) break;}

    for (int f_13 = 0; f_13 < array_static[0]; f_13++) {volatile int keep = 1; if (keep) break;}
    variable = 0;
    size_t _size_t_temp = 0;
    size_t* _size_t_temp_ptr = &_size_t_temp;
    stable_array[0] = 0;
    outer_struct.x = 0;
    for (int f_14 = 0; f_14 < stable_array[variable]; f_14++) {volatile int keep = 1; if (keep) break;}
    for (int f_15 = 0; f_15 < stable_array[_size_t_temp]; f_15++) {volatile int keep = 1; if (keep) break;}
    for (int f_16 = 0; f_16 < stable_array[stable_array[variable]]; f_16++) {volatile int keep = 1; if (keep) break;}
    for (int f_17 = 0; f_17 < stable_array[stable_array[_size_t_temp]]; f_17++)
        {volatile int keep = 1; if (keep) break;}
    for (int f_18 = 0; f_18 < stable_array[stable_array[stable_array[variable]]]; f_18++)
        {volatile int keep = 1; if (keep) break;}
    for (int f_19 = 0; f_19 < stable_array[stable_array[outer_struct.x]]; f_19++)
        {volatile int keep = 1; if (keep) break;}
    for (int f_20 = 0; f_20 < stable_array[outer_struct.x]; f_20++) {volatile int keep = 1; if (keep) break;}
    int array_temp_0[10] = {0};
    int array_temp_1[10] = {0};
    int array_temp_2[10] = {0};
    int* array_temp_0_ptr = array_temp_0;
    int* array_temp_1_ptr = &array_temp_1[1];
    int (*array_temp_2_ptr)[10] = &array_temp_2;
    for (int f_21 = 0; f_21 < array_temp_0[0]; f_21++) {volatile int keep = 1; if (keep) break;}
    for (int f_22 = 0; f_22 < array_temp_1[0]; f_22++) {volatile int keep = 1; if (keep) break;}
    for (int f_23 = 0; f_23 < array_temp_2[0]; f_23++) {volatile int keep = 1; if (keep) break;}
    int array_temp_3[10] = {0};
    int array_temp_4[10] = {0};
    int array_temp_5[10] = {0};
    for (int f_24 = 0; f_24 < array_temp_3[0]; f_24++)
    {
        int* array_temp_3_ptr = array_temp_3;
        volatile int keep = 1;
        if (keep) break;
    }
    for (int f_25 = 0; f_25 < array_temp_4[0]; f_25++)
    {
        int* array_temp_4_ptr = &array_temp_4[1];
        volatile int keep = 1;
        if (keep) break;
    }
    for (int f_26 = 0; f_26 < array_temp_5[0]; f_26++)
    {
        int (*array_temp_5_ptr)[10] = &array_temp_5;
        volatile int keep = 1;
        if (keep) break;
    }
    for (int f_27 = 0; f_27 < stable_array[0]; f_27++) {stable_array[1] = 0; volatile int keep = 1; if (keep) break;}

    for (int f_28 = 0; f_28 < outer_struct.x; f_28++) {volatile int keep = 1; if (keep) break;}
    static outer_struct_t outer_struct_static_temp = { 0, { 0 } };
    for (int f_29 = 0; f_29 < outer_struct_static_temp.inner_struct.x; f_29++) {volatile int keep = 1; if (keep) break;}
    outer_struct_t outer_struct_temp_0 = { 0, { 0 } };
    int* outer_struct_temp_0_ptr = &outer_struct_temp_0.inner_struct.x;
    for (int f_30 = 0; f_30 < outer_struct_temp_0.x; f_30++) {volatile int keep = 1; if (keep) break;}
    outer_struct_t outer_struct_temp_1 = { 0, { 0 } };
    outer_struct_t outer_struct_temp_2 = { 0, { 0 } };
    for (int f_31 = 0; f_31 < outer_struct_temp_1.x; f_31++)
    {
        int* outer_struct_temp_1_ptr = &outer_struct_temp_1.inner_struct.x;
        volatile int keep = 1;
        if (keep) break;
    }
    for (int f_32 = 0; f_32 < outer_struct_temp_2.inner_struct.x; f_32++)
    {
        outer_struct_t* outer_struct_temp_2_ptr = &outer_struct_temp_2;
        volatile int keep = 1;
        if (keep) break;
    }
    outer_struct_t outer_struct_temp_3 = { 0, { 0 } };
    outer_struct_t outer_struct_temp_4 = { 0, { 0 } };
    outer_struct_t outer_struct_temp_5 = { 0, { 0 } };
    for (int f_33 = 0; f_33 < outer_struct_temp_3.x; f_33++)
    {
        outer_struct_temp_3 = (outer_struct_t){ 0, { 0 } };
        volatile int keep = 1;
        if (keep) break;
    }
    for (int f_34 = 0; f_34 < outer_struct_temp_4.x; f_34++)
    {
        outer_struct_temp_4.inner_struct.x = 0;
        volatile int keep = 1;
        if (keep) break;
    }
    for (int f_35 = 0; f_35 < outer_struct_temp_5.inner_struct.x; f_35++)
    {
        outer_struct_temp_5.x = 0;
        volatile int keep = 1;
        if (keep) break;
    }

    stable_variable = stable_variable_const;
    for (int f_36 = 0; f_36 > stable_variable; f_36--) {volatile int keep = 1; if (keep) break;}
    stable_variable = 0;
    stable_array[0] = stable_variable_const;
    for (int f_37 = 0; stable_array[0] < f_37; f_37--) {volatile int keep = 1; if (keep) break;}
    stable_array[0] = 0;
    for (int f_38 = 0; f_38 > stable_array[stable_variable_const]; f_38--) {volatile int keep = 1; if (keep) break;}
    stable_outer_struct.x = stable_variable_const;
    for (int f_39 = 0; stable_outer_struct.x < f_39; f_39--) {volatile int keep = 1; if (keep) break;}
    stable_outer_struct.x = 0;
    for (uint32_t f_40 = 0; f_40 > UINT32_C(4294967295); f_40--) {volatile int keep = 1; if (keep) break;}
    for (int64_t f_41 = 0; INT64_MAX < f_41; f_41--) {volatile int keep = 1; if (keep) break;}
    const uint64_t uint64_max = UINT64_C(18446744073709551615);
    for (uint64_t f_42 = 0; f_42 > uint64_max; f_42--) {volatile int keep = 1; if (keep) break;}

    // decrementing_or_incrementing

    for (int i_254 = 0; i_254 < 10; i_254++) {volatile int keep = 1; if (keep) break;}
    for (int i_255 = 0; i_255 <= 10; i_255++) {volatile int keep = 1; if (keep) break;}
    for (int i_256 = 0; 10 > i_256; i_256++) {volatile int keep = 1; if (keep) break;}
    for (int i_257 = 0; 10 >= i_257; i_257++) {volatile int keep = 1; if (keep) break;}

    for (int f_43 = 0; 10 <= f_43; f_43++) {volatile int keep = 1; if (keep) break;}

    for (int i_258 = 0; i_258 > 10; i_258--) {volatile int keep = 1; if (keep) break;}
    for (int i_259 = 0; i_259 >= 10; i_259--) {volatile int keep = 1; if (keep) break;}
    for (int i_260 = 0; 10 < i_260 ; i_260--) {volatile int keep = 1; if (keep) break;}
    for (int i_261 = 0; 10 <= i_261; i_261--) {volatile int keep = 1; if (keep) break;}

    for (int f_44 = 0; 10 >= f_44; f_44--) {volatile int keep = 1; if (keep) break;}

    for (int i_262 = 0; i_262 < 10; i_262 += 1) {volatile int keep = 1; if (keep) break;}
    for (int i_263 = 0; i_263 <= 10; i_263 += 1) {volatile int keep = 1; if (keep) break;}
    for (int i_264 = 0; 10 > i_264; i_264 += 2) {volatile int keep = 1; if (keep) break;}
    for (int i_265 = 0; 10 >= i_265; i_265 += 2) {volatile int keep = 1; if (keep) break;}

    for (int f_45 = 0; 10 < f_45; f_45 += 1) {volatile int keep = 1; if (keep) break;}

    for (int i_266 = 0; i_266 > 10; i_266 -= 1) {volatile int keep = 1; if (keep) break;}
    for (int i_267 = 0; i_267 >= 10; i_267 -= 1) {volatile int keep = 1; if (keep) break;}
    for (int i_268 = 0; 10 < i_268; i_268 -= 2) {volatile int keep = 1; if (keep) break;}
    for (int i_269 = 0; 10 <= i_269; i_269 -= 2) {volatile int keep = 1; if (keep) break;}

    for (int f_46 = 0; 10 > f_46; f_46 -= 1) {volatile int keep = 1; if (keep) break;}

    for (int i_270 = 0; i_270 > 10; i_270 += -1) {volatile int keep = 1; if (keep) break;}
    for (int i_271 = 0; i_271 >= 10; i_271 += -1) {volatile int keep = 1; if (keep) break;}
    for (int i_272 = 0; 10 < i_272; i_272 += -2) {volatile int keep = 1; if (keep) break;}
    for (int i_273 = 0; 10 <= i_273; i_273 += -2) {volatile int keep = 1; if (keep) break;}

    for (int f_47 = 0; 10 >= f_47; f_47 += -2) {volatile int keep = 1; if (keep) break;}

    for (int i_274 = 0; i_274 < 10; i_274 -= -1) {volatile int keep = 1; if (keep) break;}
    for (int i_275 = 0; i_275 <= 10; i_275 -= -1) {volatile int keep = 1; if (keep) break;}
    for (int i_276 = 0; 10 > i_276; i_276 -= -2) {volatile int keep = 1; if (keep) break;}
    for (int i_277 = 0; 10 >= i_277; i_277 -= -2) {volatile int keep = 1; if (keep) break;}

    for (int f_48 = 0; 10 <= f_48; f_48 -= -2) {volatile int keep = 1; if (keep) break;}

    for (int i_278 = 0; i_278 < 10; i_278 = i_278 + 1) {volatile int keep = 1; if (keep) break;}
    for (int i_279 = 0; i_279 <= 10; i_279 = i_279 + 1) {volatile int keep = 1; if (keep) break;}
    for (int i_280 = 0; 10 > i_280; i_280 = i_280 + 2) {volatile int keep = 1; if (keep) break;}
    for (int i_281 = 0; 10 >= i_281; i_281 = i_281 + 2) {volatile int keep = 1; if (keep) break;}

    for (int f_49 = 0; 10 < f_49; f_49 = f_49 + 1) {volatile int keep = 1; if (keep) break;}

    for (int i_282 = 0; i_282 > 10; i_282 = i_282 - 1) {volatile int keep = 1; if (keep) break;}
    for (int i_283 = 0; i_283 >= 10; i_283 = i_283 - 1) {volatile int keep = 1; if (keep) break;}
    for (int i_284 = 0; 10 < i_284; i_284 = i_284 - 2) {volatile int keep = 1; if (keep) break;}
    for (int i_285 = 0; 10 <= i_285; i_285 = i_285 - 2) {volatile int keep = 1; if (keep) break;}

    for (int f_50 = 0; 10 > f_50; f_50 = f_50 - 1) {volatile int keep = 1; if (keep) break;}

    for (int i_286 = 0; i_286 > 10; i_286 = i_286 + -1) {volatile int keep = 1; if (keep) break;}
    for (int i_287 = 0; i_287 >= 10; i_287 = i_287 + -1) {volatile int keep = 1; if (keep) break;}
    for (int i_288 = 0; 10 < i_288; i_288 = i_288 + -2) {volatile int keep = 1; if (keep) break;}
    for (int i_289 = 0; 10 <= i_289; i_289 = i_289 + -2) {volatile int keep = 1; if (keep) break;}

    for (int f_51 = 0; 10 >= f_51; f_51 = f_51 + -2) {volatile int keep = 1; if (keep) break;}

    for (int i_290 = 0; i_290 < 10; i_290 = i_290 - -1) {volatile int keep = 1; if (keep) break;}
    for (int i_291 = 0; i_291 <= 10; i_291 = i_291 - -1) {volatile int keep = 1; if (keep) break;}
    for (int i_292 = 0; 10 > i_292; i_292 = i_292 - -2) {volatile int keep = 1; if (keep) break;}
    for (int i_293 = 0; 10 >= i_293; i_293 = i_293 - -2) {volatile int keep = 1; if (keep) break;}

    for (int f_52 = 0; 10 <= f_52; f_52 = f_52 - -2) {volatile int keep = 1; if (keep) break;}

    for (int i_294 = 0; i_294 < 10; i_294 = 1 + i_294) {volatile int keep = 1; if (keep) break;}
    for (int i_295 = 0; i_295 <= 10; i_295 = 1 + i_295) {volatile int keep = 1; if (keep) break;}
    for (int i_296 = 0; 10 > i_296; i_296 = 2 + i_296) {volatile int keep = 1; if (keep) break;}
    for (int i_297 = 0; 10 >= i_297; i_297 = 2 + i_297) {volatile int keep = 1; if (keep) break;}

    for (int f_53 = 0; 10 < f_53; f_53 = 1 + f_53) {volatile int keep = 1; if (keep) break;}

    for (int i_298 = 0; i_298 > 10; i_298 = -1 + i_298) {volatile int keep = 1; if (keep) break;}
    for (int i_299 = 0; i_299 >= 10; i_299 = -1 + i_299) {volatile int keep = 1; if (keep) break;}
    for (int i_300 = 0; 10 < i_300; i_300 = -2 + i_300) {volatile int keep = 1; if (keep) break;}
    for (int i_301 = 0; 10 <= i_301; i_301 = -2 + i_301) {volatile int keep = 1; if (keep) break;}

    for (int f_54 = 0; 10 > f_54; f_54 = -1 + f_54) {volatile int keep = 1; if (keep) break;}

    for (unsigned int i_302 = 0; i_302 < 10; i_302++) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_303 = 0; i_303 <= 10; i_303++) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_304 = 0; 10 > i_304; i_304++) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_305 = 0; 10 >= i_305; i_305++) {volatile int keep = 1; if (keep) break;}

    for (unsigned int f_55 = 0; 10 <= f_55; f_55++) {volatile int keep = 1; if (keep) break;}

    for (unsigned int i_306 = 0; i_306 > 10; i_306--) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_307 = 0; i_307 >= 10; i_307--) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_308 = 0; 10 < i_308 ; i_308--) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_309 = 0; 10 <= i_309; i_309--) {volatile int keep = 1; if (keep) break;}

    for (unsigned int f_56 = 0; 10 >= f_56; f_56--) {volatile int keep = 1; if (keep) break;}

    for (unsigned int i_310 = 0; i_310 < 10; i_310 += 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_311 = 0; i_311 <= 10; i_311 += 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_312 = 0; 10 > i_312; i_312 += 2) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_313 = 0; 10 >= i_313; i_313 += 2) {volatile int keep = 1; if (keep) break;}

    for (unsigned int f_57 = 0; 10 < f_57; f_57 += 1) {volatile int keep = 1; if (keep) break;}

    for (unsigned int i_314 = 0; i_314 > 10; i_314 -= 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_315 = 0; i_315 >= 10; i_315 -= 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_316 = 0; 10 < i_316; i_316 -= 2) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_317 = 0; 10 <= i_317; i_317 -= 2) {volatile int keep = 1; if (keep) break;}

    for (unsigned int f_58 = 0; 10 > f_58; f_58 -= 1) {volatile int keep = 1; if (keep) break;}

    for (unsigned int i_318 = 0; i_318 < 10; i_318 = i_318 + 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_319 = 0; i_319 <= 10; i_319 = i_319 + 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_320 = 0; 10 > i_320; i_320 = i_320 + 2) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_321 = 0; 10 >= i_321; i_321 = i_321 + 2) {volatile int keep = 1; if (keep) break;}

    for (unsigned int f_59 = 0; 10 <= f_59; f_59 = f_59 + 2) {volatile int keep = 1; if (keep) break;}

    for (unsigned int i_322 = 0; i_322 > 10; i_322 = i_322 - 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_323 = 0; i_323 >= 10; i_323 = i_323 - 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_324 = 0; 10 < i_324; i_324 = i_324 - 2) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_325 = 0; 10 <= i_325; i_325 = i_325 - 2) {volatile int keep = 1; if (keep) break;}

    for (unsigned int f_60 = 0; 10 >= f_60; f_60 = f_60 - 2) {volatile int keep = 1; if (keep) break;}

    for (unsigned int i_326 = 0; i_326 < 10; i_326 = 1 + i_326) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_327 = 0; i_327 <= 10; i_327 = 1 + i_327) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_328 = 0; 10 > i_328; i_328 = 2 + i_328) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_329 = 0; 10 >= i_329; i_329 = 2 + i_329) {volatile int keep = 1; if (keep) break;}

    for (unsigned int f_61 = 0; 10 < f_61; f_61 = 1 + f_61) {volatile int keep = 1; if (keep) break;}

    _signed_char = 1;
    for (int i_330 = 0; i_330 < 10; i_330 += _signed_char) {volatile int keep = 1; if (keep) break;}
    for (int i_331 = 0; i_331 <= 10; i_331 += _signed_char) {volatile int keep = 1; if (keep) break;}
    for (int i_332 = 0; 10 > i_332; i_332 += _signed_char) {volatile int keep = 1; if (keep) break;}
    for (int i_333 = 0; 10 >= i_333; i_333 += _signed_char) {volatile int keep = 1; if (keep) break;}

    for (int f_62 = 0; 10 < f_62; f_62 += _signed_char) {volatile int keep = 1; if (keep) break;}

    for (int i_334 = 0; i_334 > 10; i_334 -= _signed_char) {volatile int keep = 1; if (keep) break;}
    for (int i_335 = 0; i_335 >= 10; i_335 -= _signed_char) {volatile int keep = 1; if (keep) break;}
    for (int i_336 = 0; 10 < i_336; i_336 -= _signed_char) {volatile int keep = 1; if (keep) break;}
    for (int i_337 = 0; 10 <= i_337; i_337 -= _signed_char) {volatile int keep = 1; if (keep) break;}

    for (int f_63 = 0; 10 >= f_63; f_63 -= _signed_char) {volatile int keep = 1; if (keep) break;}

    _short = -1;
    for (int i_338 = 0; i_338 > 10; i_338 += _short) {volatile int keep = 1; if (keep) break;}
    for (int i_339 = 0; i_339 >= 10; i_339 += _short) {volatile int keep = 1; if (keep) break;}
    for (int i_340 = 0; 10 < i_340; i_340 += _short) {volatile int keep = 1; if (keep) break;}
    for (int i_341 = 0; 10 <= i_341; i_341 += _short) {volatile int keep = 1; if (keep) break;}

    for (int f_64 = 0; 10 >= f_64; f_64 += _short) {volatile int keep = 1; if (keep) break;}

    for (int i_342 = 0; i_342 < 10; i_342 -= _short) {volatile int keep = 1; if (keep) break;}
    for (int i_343 = 0; i_343 <= 10; i_343 -= _short) {volatile int keep = 1; if (keep) break;}
    for (int i_344 = 0; 10 > i_344; i_344 -= _short) {volatile int keep = 1; if (keep) break;}
    for (int i_345 = 0; 10 >= i_345; i_345 -= _short) {volatile int keep = 1; if (keep) break;}

    for (int f_65 = 0; 10 < f_65; f_65 -= _short) {volatile int keep = 1; if (keep) break;}

    stable_variable = 1;
    for (int i_346 = 0; i_346 < 10; i_346 = i_346 + stable_variable) {volatile int keep = 1; if (keep) break;}
    for (int i_347 = 0; i_347 <= 10; i_347 = i_347 + stable_variable) {volatile int keep = 1; if (keep) break;}
    for (int i_348 = 0; 10 > i_348; i_348 = i_348 + stable_variable) {volatile int keep = 1; if (keep) break;}
    for (int i_349 = 0; 10 >= i_349; i_349 = i_349 + stable_variable) {volatile int keep = 1; if (keep) break;}

    for (int f_66 = 0; 10 < f_66; f_66 = f_66 + stable_variable) {volatile int keep = 1; if (keep) break;}
    variable = 1;
    for (int f_67 = 0; 10 >= f_67; f_67 = f_67 + variable) {volatile int keep = 1; if (keep) break;}

    for (int i_350 = 0; i_350 > 10; i_350 = i_350 - stable_variable) {volatile int keep = 1; if (keep) break;}
    for (int i_351 = 0; i_351 >= 10; i_351 = i_351 - stable_variable) {volatile int keep = 1; if (keep) break;}
    for (int i_352 = 0; 10 < i_352; i_352 = i_352 - stable_variable) {volatile int keep = 1; if (keep) break;}
    for (int i_353 = 0; 10 <= i_353; i_353 = i_353 - stable_variable) {volatile int keep = 1; if (keep) break;}

    for (int f_68 = 0; 10 >= f_68; f_68 = f_68 - stable_variable) {volatile int keep = 1; if (keep) break;}

    _int8_t = -2;
    for (int i_354 = 0; i_354 > 10; i_354 = i_354 + _int8_t) {volatile int keep = 1; if (keep) break;}
    for (int i_355 = 0; i_355 >= 10; i_355 = i_355 + _int8_t) {volatile int keep = 1; if (keep) break;}
    for (int i_356 = 0; 10 < i_356; i_356 = i_356 + _int8_t) {volatile int keep = 1; if (keep) break;}
    for (int i_357 = 0; 10 <= i_357; i_357 = i_357 + _int8_t) {volatile int keep = 1; if (keep) break;}

    for (int f_69 = 0; 10 >= f_69; f_69 = f_69 + _int8_t) {volatile int keep = 1; if (keep) break;}

    for (int i_358 = 0; i_358 < 10; i_358 = i_358 - _int8_t) {volatile int keep = 1; if (keep) break;}
    for (int i_359 = 0; i_359 <= 10; i_359 = i_359 - _int8_t) {volatile int keep = 1; if (keep) break;}
    for (int i_360 = 0; 10 > i_360; i_360 = i_360 - _int8_t) {volatile int keep = 1; if (keep) break;}
    for (int i_361 = 0; 10 >= i_361; i_361 = i_361 - _int8_t) {volatile int keep = 1; if (keep) break;}

    for (int f_70 = 0; 10 < f_70; f_70 = f_70 - _int8_t) {volatile int keep = 1; if (keep) break;}

    _int16_t = 2;
    for (int i_362 = 0; i_362 < 10; i_362 = _int16_t + i_362) {volatile int keep = 1; if (keep) break;}
    for (int i_363 = 0; i_363 <= 10; i_363 = _int16_t + i_363) {volatile int keep = 1; if (keep) break;}
    for (int i_364 = 0; 10 > i_364; i_364 = _int16_t + i_364) {volatile int keep = 1; if (keep) break;}
    for (int i_365 = 0; 10 >= i_365; i_365 = _int16_t + i_365) {volatile int keep = 1; if (keep) break;}

    for (int f_71 = 0; 10 < f_71; f_71 = _int16_t + f_71) {volatile int keep = 1; if (keep) break;}

    _typedef_int16_t = -2;
    for (int i_366 = 0; i_366 > 10; i_366 = _typedef_int16_t + i_366) {volatile int keep = 1; if (keep) break;}
    for (int i_367 = 0; i_367 >= 10; i_367 = _typedef_int16_t + i_367) {volatile int keep = 1; if (keep) break;}
    for (int i_368 = 0; 10 < i_368; i_368 = _typedef_int16_t + i_368) {volatile int keep = 1; if (keep) break;}
    for (int i_369 = 0; 10 <= i_369; i_369 = _typedef_int16_t + i_369) {volatile int keep = 1; if (keep) break;}

    for (int f_72 = 0; 10 >= f_72; f_72 = _typedef_int16_t + f_72) {volatile int keep = 1; if (keep) break;}

    #define VARIABLE_GLOBAL_TEMP 1
    const int stable_variable_const_temp = -2;
    stable_array[0] = 3;
    const int stable_array_const_temp[10] = {-4};
    stable_array_2d[0][0] = 5;
    stable_outer_struct.x = -6;
    const outer_struct_t stable_outer_struct_const_temp = { 0, { 7 } };
    enum { type_alias_value_temp = -8 };
    for (int i_370 = 0; i_370 < 10; i_370 += VARIABLE_GLOBAL_TEMP) {volatile int keep = 1; if (keep) break;}
    for (int i_371 = 0; i_371 < 10; i_371 -= stable_variable_const_temp) {volatile int keep = 1; if (keep) break;}
    for (int i_372 = 0; i_372 < 10; i_372 = i_372 +  stable_array[0]) {volatile int keep = 1; if (keep) break;}
    for (int i_373 = 0; i_373 < 10; i_373 = i_373 -  stable_array_const_temp[0]) {volatile int keep = 1; if (keep) break;}
    for (int i_374 = 0; i_374 > 10; i_374 -= stable_array_2d[0][0]) {volatile int keep = 1; if (keep) break;}
    for (int i_375 = 0; i_375 > 10; i_375 += stable_outer_struct.x) {volatile int keep = 1; if (keep) break;}
    for (int i_376 = 0; i_376 > 10; i_376 = i_376 - stable_outer_struct_const_temp.inner_struct.x)
        {volatile int keep = 1; if (keep) break;}
    for (int i_377 = 0; i_377 > 10; i_377 = i_377 + type_alias_value_temp) {volatile int keep = 1; if (keep) break;}

    stable_variable = stable_variable_const;
    for (int f_73 = 0; f_73 < 10; f_73 += stable_variable) {volatile int keep = 1; if (keep) break;}
    stable_variable = 0;
    stable_array[0] = stable_variable_const;
    for (int f_74 = 0; f_74 < 10; f_74 += stable_array[0]) {volatile int keep = 1; if (keep) break;}
    stable_array[0] = 0;
    for (int f_75 = 0; f_75 < 10; f_75 += stable_array[stable_variable_const]) {volatile int keep = 1; if (keep) break;}
    stable_outer_struct.x = stable_variable_const;
    for (int f_76 = 0; f_76 < 10; f_76 += stable_outer_struct.x) {volatile int keep = 1; if (keep) break;}
    stable_outer_struct.x = 0;
    for (int f_77 = 0; f_77 < 10; f_77 += VARIABLE_GLOBAL_CONST) {volatile int keep = 1; if (keep) break;}
    for (int f_78 = 0; f_78 < 10; f_78 += stable_variable_const) {volatile int keep = 1; if (keep) break;}
    for (int f_79 = 0; f_79 < 10; f_79 += stable_array_const[0]) {volatile int keep = 1; if (keep) break;}
    for (int f_80 = 0; f_80 < 10; f_80 += stable_outer_struct_const.x) {volatile int keep = 1; if (keep) break;}
    for (int f_81 = 0; f_81 < 10; f_81 += stable_outer_struct_const.inner_struct.x)
        {volatile int keep = 1; if (keep) break;}
    for (int f_82 = 0; f_82 < 10; f_82 += type_alias_value) {volatile int keep = 1; if (keep) break;}

    // body

    for (int f_83 = 0; f_83 < 10; f_83++) {f_83++; volatile int keep = 1; if (keep) break;}

    // END

    printf("End of test_3_idiomatize_loops_initial_transformation().\n");
}

static int return_int()
{
    return 0;
}

static unsigned char return_unsigned_char()
{
    return 0;
}

static long long return_long_long()
{
    return 0;
}

static int8_t return_int8_t()
{
    return 0;
}

static uint64_t return_uint64_t()
{
    return 0;
}

static intptr_t return_intptr_t()
{
    return 0;
}

static uintptr_t return_uintptr_t()
{
    return 0;
}

static size_t return_size_t()
{
    return 0;
}

static typedef_unsigned_char return_typedef_unsigned_char()
{
    return 0;
}

static typedef_size_t return_typedef_size_t()
{
    return 0;
}

static void receive_t_values_for_l
(
    int variable,
    const int variable_const,
    int* pointer,
    const int* pointer_const_0,
    int* const pointer_const_1,
    int** pointer_2d,
    outer_struct_t outer_struct,
    const outer_struct_t outer_struct_const,
    outer_struct_t* outer_struct_ptr,
    const outer_struct_t* outer_struct_ptr_const_0,
    outer_struct_t* const outer_struct_ptr_const_1
)
{
    for (int i_378 = variable; i_378 < 10; i_378++) {volatile int keep = 1; if (keep) break;}
    for (int i_379 = variable_const; i_379 < 10; i_379++) {volatile int keep = 1; if (keep) break;}
    for (int i_380 = *pointer; i_380 < 10; i_380++) {volatile int keep = 1; if (keep) break;}
    for (int i_381 = *pointer_const_0; i_381 < 10; i_381++) {volatile int keep = 1; if (keep) break;}
    for (int i_382 = *pointer_const_1; i_382 < 10; i_382++) {volatile int keep = 1; if (keep) break;}
    for (int i_383 = **pointer_2d; i_383 < 10; i_383++) {volatile int keep = 1; if (keep) break;}
    for (int i_384 = outer_struct.x; i_384 < 10; i_384++) {volatile int keep = 1; if (keep) break;}
    for (int i_385 = outer_struct.inner_struct.x; i_385 < 10; i_385++) {volatile int keep = 1; if (keep) break;}
    for (int i_386 = outer_struct_const.x; i_386 < 10; i_386++) {volatile int keep = 1; if (keep) break;}
    for (int i_387 = outer_struct_const.inner_struct.x; i_387 < 10; i_387++) {volatile int keep = 1; if (keep) break;}
    for (int i_388 = outer_struct_ptr->x; i_388 < 10; i_388++) {volatile int keep = 1; if (keep) break;}
    for (int i_389 = outer_struct_ptr->inner_struct.x; i_389 < 10; i_389++) {volatile int keep = 1; if (keep) break;}
    for (int i_390 = outer_struct_ptr_const_0->x; i_390 < 10; i_390++) {volatile int keep = 1; if (keep) break;}
    for (int i_391 = outer_struct_ptr_const_0->inner_struct.x; i_391 < 10; i_391++)
        {volatile int keep = 1; if (keep) break;}
    for (int i_392 = outer_struct_ptr_const_1->x; i_392 < 10; i_392++) {volatile int keep = 1; if (keep) break;}
    for (int i_393 = outer_struct_ptr_const_1->inner_struct.x; i_393 < 10; i_393++)
        {volatile int keep = 1; if (keep) break;}
}

static void receive_stable_t_values_for_n
(
    int stable_variable,
    const int stable_variable_const,
    outer_struct_t stable_outer_struct,
    const outer_struct_t stable_outer_struct_const
)
{
    for (int i_394 = 0; i_394 < stable_variable; i_394++) {volatile int keep = 1; if (keep) break;}
    for (int i_395 = 0; i_395 < stable_variable_const; i_395++) {volatile int keep = 1; if (keep) break;}
    for (int i_396 = 0; i_396 < stable_outer_struct.x; i_396++) {volatile int keep = 1; if (keep) break;}
    for (int i_397 = 0; i_397 < stable_outer_struct.inner_struct.x; i_397++) {volatile int keep = 1; if (keep) break;}
    for (int i_398 = 0; i_398 < stable_outer_struct_const.x; i_398++) {volatile int keep = 1; if (keep) break;}
    for (int i_399 = 0; i_399 < stable_outer_struct_const.inner_struct.x; i_399++)
        {volatile int keep = 1; if (keep) break;}

    int* stable_variable_ptr = &stable_variable;
    for (int f_84 = 0; f_84 < stable_variable; f_84++) {volatile int keep = 1; if (keep) break;}
    outer_struct_t* stable_outer_struct_ptr = &stable_outer_struct;
    for (int f_85 = 0; f_85 < stable_outer_struct.x; f_85++) {volatile int keep = 1; if (keep) break;}
}

static void receive_statically_determinable_t_values_for_n
(
    int stable_variable,
    outer_struct_t stable_outer_struct
)
{
    for (int f_86 = 0; f_86 > stable_variable; f_86--) {volatile int keep = 1; if (keep) break;}
    for (int f_87 = 0; stable_outer_struct.x < f_87; f_87--) {volatile int keep = 1; if (keep) break;}
    for (int f_88 = 0; f_88 > stable_outer_struct.inner_struct.x; f_88--) {volatile int keep = 1; if (keep) break;}

    stable_variable = 0;
    stable_outer_struct.x = 0;
    stable_outer_struct.inner_struct.x = 0;
    for (int i_400 = 0; i_400  > stable_variable; i_400 --) {volatile int keep = 1; if (keep) break;}
    for (int i_401 = 0; stable_outer_struct.x < i_401; i_401 --) {volatile int keep = 1; if (keep) break;}
    for (int i_402 = 0; i_402 > stable_outer_struct.inner_struct.x; i_402 --)
        {volatile int keep = 1; if (keep) break;}
}
