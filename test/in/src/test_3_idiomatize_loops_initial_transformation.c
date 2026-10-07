#include <stdint.h>
#include <stdio.h>

#include "test_3_idiomatize_loops_initial_transformation.h"
#include "nested_struct.h"

static int return_int();

static unsigned char return_unsigned_char();

static long long return_long_long();

static int8_t return_int8_t();

static uint64_t return_uint64_t();

static intptr_t return_intptr_t();

static uintptr_t return_uintptr_t();

static size_t return_size_t();

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
    for (intptr_t i_11 = 0; i_11 < 10; i_11++) {volatile int keep = 1; if (keep) break;}
    for (uintptr_t i_12 = 0; i_12 < 10; i_12++) {volatile int keep = 1; if (keep) break;}
    for (size_t i_13 = 0; i_13 < 10; i_13++) {volatile int keep = 1; if (keep) break;}

    for (char f_2 = 0; f_2 < 10; f_2++) {volatile int keep = 1; if (keep) break;}

    // L

    for (int i_14 = 0; i_14 < 10; i_14++) {volatile int keep = 1; if (keep) break;}
    for (int i_15 = -3; i_15 < 10; i_15++) {volatile int keep = 1; if (keep) break;}
    for (int i_16 = 6; i_16 < 10; i_16++) {volatile int keep = 1; if (keep) break;}
    for (int i_17 = -011; i_17 < 10; i_17++) {volatile int keep = 1; if (keep) break;}
    for (int i_18 = 0xC; i_18 < 10; i_18++) {volatile int keep = 1; if (keep) break;}

    for (int i_19 = variable_global; i_19 < 10; i_19++) {volatile int keep = 1; if (keep) break;}
    for (int i_20 = VARIABLE_GLOBAL_CONST; i_20 < 10; i_20++) {volatile int keep = 1; if (keep) break;}
    for (int i_21 = variable_global_static; i_21 < 10; i_21++) {volatile int keep = 1; if (keep) break;}
    for (int i_22 = variable; i_22 < 10; i_22++) {volatile int keep = 1; if (keep) break;}
    for (int i_23 = variable_const; i_23 < 10; i_23++) {volatile int keep = 1; if (keep) break;}
    for (int i_24 = variable_static; i_24 < 10; i_24++) {volatile int keep = 1; if (keep) break;}

    for (int i_25 = *pointer_global; i_25 < 10; i_25++) {volatile int keep = 1; if (keep) break;}
    for (int i_26 = *pointer_global_const_0; i_26 < 10; i_26++) {volatile int keep = 1; if (keep) break;}
    for (int i_27 = *pointer_global_static; i_27 < 10; i_27++) {volatile int keep = 1; if (keep) break;}
    for (int i_28 = **pointer_2d_global; i_28 < 10; i_28++) {volatile int keep = 1; if (keep) break;}
    for (int i_29 = *pointer; i_29 < 10; i_29++) {volatile int keep = 1; if (keep) break;}
    for (int i_30 = *pointer_const_0; i_30 < 10; i_30++) {volatile int keep = 1; if (keep) break;}
    for (int i_31 = *pointer_const_1; i_31 < 10; i_31++) {volatile int keep = 1; if (keep) break;}
    for (int i_32 = *pointer_static; i_32 < 10; i_32++) {volatile int keep = 1; if (keep) break;}
    for (int i_33 = **pointer_2d; i_33 < 10; i_33++) {volatile int keep = 1; if (keep) break;}

    const int i = 0;
    for (int i_34 = array_global_1[0]; i_34 < 10; i_34++) {volatile int keep = 1; if (keep) break;}
    for (int i_35 = array_global_1[i]; i_35 < 10; i_35++) {volatile int keep = 1; if (keep) break;}
    for (int i_36 = array_global_static[0]; i_36 < 10; i_36++) {volatile int keep = 1; if (keep) break;}
    for (int i_37 = array_global_static[i]; i_37 < 10; i_37++) {volatile int keep = 1; if (keep) break;}
    for (int i_38 = array_2d_global[0][0]; i_38 < 10; i_38++) {volatile int keep = 1; if (keep) break;}
    for (int i_39 = array_2d_global[i][i]; i_39 < 10; i_39++) {volatile int keep = 1; if (keep) break;}
    for (int i_40 = array[0]; i_40 < 10; i_40++) {volatile int keep = 1; if (keep) break;}
    for (int i_41 = array[i]; i_41 < 10; i_41++) {volatile int keep = 1; if (keep) break;}
    for (int i_42 = array_const[0]; i_42 < 10; i_42++) {volatile int keep = 1; if (keep) break;}
    for (int i_43 = array_const[i]; i_43 < 10; i_43++) {volatile int keep = 1; if (keep) break;}
    for (int i_44 = array_static[0]; i_44 < 10; i_44++) {volatile int keep = 1; if (keep) break;}
    for (int i_45 = array_static[i]; i_45 < 10; i_45++) {volatile int keep = 1; if (keep) break;}
    for (int i_46 = array_2d[0][0]; i_46 < 10; i_46++) {volatile int keep = 1; if (keep) break;}
    for (int i_47 = array_2d[i][i]; i_47 < 10; i_47++) {volatile int keep = 1; if (keep) break;}

    for (int i_48 = outer_struct_global.x; i_48 < 10; i_48++) {volatile int keep = 1; if (keep) break;}
    for (int i_49 = outer_struct_global.inner_struct.x; i_49 < 10; i_49++) {volatile int keep = 1; if (keep) break;}
    for (int i_50 = outer_struct_global_static.x; i_50 < 10; i_50++) {volatile int keep = 1; if (keep) break;}
    for (int i_51 = outer_struct_global_static.inner_struct.x; i_51 < 10; i_51++)
        {volatile int keep = 1; if (keep) break;}
    for (int i_52 = outer_struct.x; i_52 < 10; i_52++) {volatile int keep = 1; if (keep) break;}
    for (int i_53 = outer_struct.inner_struct.x; i_53 < 10; i_53++) {volatile int keep = 1; if (keep) break;}
    for (int i_54 = outer_struct_const.x; i_54 < 10; i_54++) {volatile int keep = 1; if (keep) break;}
    for (int i_55 = outer_struct_const.inner_struct.x; i_55 < 10; i_55++) {volatile int keep = 1; if (keep) break;}
    for (int i_56 = outer_struct_static.x; i_56 < 10; i_56++) {volatile int keep = 1; if (keep) break;}
    for (int i_57 = outer_struct_static.inner_struct.x; i_57 < 10; i_57++) {volatile int keep = 1; if (keep) break;}

    for (int i_58 = outer_struct_ptr_global->x; i_58 < 10; i_58++) {volatile int keep = 1; if (keep) break;}
    for (int i_59 = outer_struct_ptr_global->inner_struct.x; i_59 < 10; i_59++)
        {volatile int keep = 1; if (keep) break;}
    for (int i_60 = outer_struct_ptr_global_const_0->x; i_60 < 10; i_60++) {volatile int keep = 1; if (keep) break;}
    for (int i_61 = outer_struct_ptr_global_const_0->inner_struct.x; i_61 < 10; i_61++)
        {volatile int keep = 1; if (keep) break;}
    for (int i_62 = outer_struct_ptr_global_static->x; i_62 < 10; i_62++)
        {volatile int keep = 1; if (keep) break;}
    for (int i_63 = outer_struct_ptr_global_static->inner_struct.x; i_63 < 10; i_63++)
        {volatile int keep = 1; if (keep) break;}
    for (int i_64 = outer_struct_ptr->x; i_64 < 10; i_64++) {volatile int keep = 1; if (keep) break;}
    for (int i_65 = outer_struct_ptr->inner_struct.x; i_65 < 10; i_65++) {volatile int keep = 1; if (keep) break;}
    for (int i_66 = outer_struct_ptr_const_0->x; i_66 < 10; i_66++) {volatile int keep = 1; if (keep) break;}
    for (int i_67 = outer_struct_ptr_const_0->inner_struct.x; i_67 < 10; i_67++)
        {volatile int keep = 1; if (keep) break;}
    for (int i_68 = outer_struct_ptr_const_1->x; i_68 < 10; i_68++) {volatile int keep = 1; if (keep) break;}
    for (int i_69 = outer_struct_ptr_const_1->inner_struct.x; i_69 < 10; i_69++)
        {volatile int keep = 1; if (keep) break;}
    for (int i_70 = outer_struct_ptr_static->x; i_70 < 10; i_70++) {volatile int keep = 1; if (keep) break;}
    for (int i_71 = outer_struct_ptr_static->inner_struct.x; i_71 < 10; i_71++)
        {volatile int keep = 1; if (keep) break;}
    for (int i_72 = return_int(); i_72 < 10; i_72++) {volatile int keep = 1; if (keep) break;}

    for (int i_73 = type_alias_value; i_73 < 10; i_73++) {volatile int keep = 1; if (keep) break;}

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

    for (int i_74 = _signed_char; i_74 < 10; i_74++) {volatile int keep = 1; if (keep) break;}
    for (int i_75 = _unsigned_char; i_75 < 10; i_75++) {volatile int keep = 1; if (keep) break;}
    for (int i_76 = _short; i_76 < 10; i_76++) {volatile int keep = 1; if (keep) break;}
    for (int i_77 = _unsigned_short; i_77 < 10; i_77++) {volatile int keep = 1; if (keep) break;}
    for (int i_78 = _int; i_78 < 10; i_78++) {volatile int keep = 1; if (keep) break;}
    for (int i_79 = _unsigned_int; i_79 < 10; i_79++) {volatile int keep = 1; if (keep) break;}
    for (int i_80 = _long; i_80 < 10; i_80++) {volatile int keep = 1; if (keep) break;}
    for (int i_81 = _unsigned_long; i_81 < 10; i_81++) {volatile int keep = 1; if (keep) break;}
    for (int i_82 = _long_long; i_82 < 10; i_82++) {volatile int keep = 1; if (keep) break;}
    for (int i_83 = _unsigned_long_long; i_83 < 10; i_83++) {volatile int keep = 1; if (keep) break;}
    for (int i_84 = _int8_t; i_84 < 10; i_84++) {volatile int keep = 1; if (keep) break;}
    for (int i_85 = _uint8_t; i_85 < 10; i_85++) {volatile int keep = 1; if (keep) break;}
    for (int i_86 = _int16_t; i_86 < 10; i_86++) {volatile int keep = 1; if (keep) break;}
    for (int i_87 = _uint16_t; i_87 < 10; i_87++) {volatile int keep = 1; if (keep) break;}
    for (int i_88 = _uint32_t; i_88 < 10; i_88++) {volatile int keep = 1; if (keep) break;}
    for (int i_89 = _int64_t; i_89 < 10; i_89++) {volatile int keep = 1; if (keep) break;}
    for (int i_90 = _uint64_t; i_90 < 10; i_90++) {volatile int keep = 1; if (keep) break;}
    for (int i_91 = _intptr_t; i_91 < 10; i_91++) {volatile int keep = 1; if (keep) break;}
    for (int i_92 = _uintptr_t; i_92 < 10; i_92++) {volatile int keep = 1; if (keep) break;}
    for (int i_93 = _size_t; i_93 < 10; i_93++) {volatile int keep = 1; if (keep) break;}

    for (int i_94 = return_unsigned_char(); i_94 < 10; i_94++)  {volatile int keep = 1; if (keep) break;}
    for (int i_95 = return_long_long(); i_95 < 10; i_95++)  {volatile int keep = 1; if (keep) break;}
    for (int i_96 = return_int8_t(); i_96 < 10; i_96++)  {volatile int keep = 1; if (keep) break;}
    for (int i_97 = return_uint64_t(); i_97 < 10; i_97++)  {volatile int keep = 1; if (keep) break;}
    for (int i_98 = return_intptr_t(); i_98 < 10; i_98++)  {volatile int keep = 1; if (keep) break;}
    for (int i_99 = return_uintptr_t(); i_99 < 10; i_99++)  {volatile int keep = 1; if (keep) break;}
    for (int i_100 = return_size_t(); i_100 < 10; i_100++)  {volatile int keep = 1; if (keep) break;}

    // relational_operator

    for (int i_101 = 0; i_101 < 10; i_101++) {volatile int keep = 1; if (keep) break;}
    for (int i_102 = 0; i_102 <= 10; i_102++) {volatile int keep = 1; if (keep) break;}
    for (int i_103 = 0; i_103 > 10; i_103--) {volatile int keep = 1; if (keep) break;}
    for (int i_104 = 0; i_104 >= 10; i_104--) {volatile int keep = 1; if (keep) break;}
    for (int i_105 = 0; 10 < i_105; i_105--) {volatile int keep = 1; if (keep) break;}
    for (int i_106 = 0; 10 <= i_106; i_106--) {volatile int keep = 1; if (keep) break;}
    for (int i_107 = 0; 10 > i_107; i_107++) {volatile int keep = 1; if (keep) break;}
    for (int i_108 = 0; 10 >= i_108; i_108++) {volatile int keep = 1; if (keep) break;}

    for (int f_3 = 0; f_3 != 10; f_3++) {volatile int keep = 1; if (keep) break;}

    // N

    for (int i_109 = 0; i_109 < 0; i_109++) {volatile int keep = 1; if (keep) break;}
    for (int i_110 = 0; i_110 < -3; i_110++) {volatile int keep = 1; if (keep) break;}
    for (int i_111 = 0; i_111 < 6; i_111++) {volatile int keep = 1; if (keep) break;}
    for (int i_112 = 0; i_112 < -011; i_112++) {volatile int keep = 1; if (keep) break;}
    for (int i_113 = 0; i_113 < 0xC; i_113++) {volatile int keep = 1; if (keep) break;}

    for (int i_114 = 0; i_114 < VARIABLE_GLOBAL_CONST; i_114++) {volatile int keep = 1; if (keep) break;}
    for (int i_115 = 0; i_115 < stable_variable; i_115++) {volatile int keep = 1; if (keep) break;}
    for (int i_116 = 0; i_116 < stable_variable_const; i_116++) {volatile int keep = 1; if (keep) break;}
    int stable_variable_temp_0 = 0;
    for (int i_117 = 0; i_117 < stable_variable_temp_0; i_117++) {volatile int keep = 1; if (keep) break;}
    int* stable_variable_temp_0_ptr = &stable_variable_temp_0;

    stable_variable = 0;
    _size_t = 0;
    stable_array[0] = 0;
    stable_outer_struct.x = 0;
    for (int i_118 = 0; i_118 < stable_array[0]; i_118++) {volatile int keep = 1; if (keep) break;}
    for (int i_119 = 0; i_119 < stable_array[stable_variable]; i_119++) {volatile int keep = 1; if (keep) break;}
    for (int i_120 = 0; i_120 < stable_array[_size_t]; i_120++) {volatile int keep = 1; if (keep) break;}
    for (int i_121 = 0; i_121 < stable_array[stable_array[0]]; i_121++) {volatile int keep = 1; if (keep) break;}
    for (int i_122 = 0; i_122 < stable_array[stable_array[stable_variable]]; i_122++)
    {
        volatile int keep = 1;
        if (keep) break;
    }
    for (int i_123 = 0; i_123 < stable_array[stable_array[_size_t]]; i_123++)
        {volatile int keep = 1; if (keep) break;}
    for (int i_124 = 0; i_124 < stable_array[stable_array[stable_array[0]]]; i_124++)
        {volatile int keep = 1; if (keep) break;}
    for (int i_125 = 0; i_125 < stable_array[stable_array[stable_outer_struct.x]]; i_125++)
        {volatile int keep = 1; if (keep) break;}
    for (int i_126 = 0; i_126 < stable_array[stable_outer_struct.x]; i_126++)
        {volatile int keep = 1; if (keep) break;}
    for (int i_127 = 0; i_127 < stable_array_const[0]; i_127++) {volatile int keep = 1; if (keep) break;}
    for (int i_128 = 0; i_128 < stable_array_2d[0][0]; i_128++) {volatile int keep = 1; if (keep) break;}
    for (int i_129 = 0; i_129 < stable_array_2d[stable_variable][stable_variable]; i_129++)
        {volatile int keep = 1; if (keep) break;}
    for (int i_130 = 0; i_130 < stable_array_2d[_size_t][_size_t]; i_130++) {volatile int keep = 1; if (keep) break;}
    for (int i_131 = 0; i_131 < stable_array_2d[stable_array[0]][stable_array[0]]; i_131++)
        {volatile int keep = 1; if (keep) break;}
    for (
        int i_132 = 0;
        i_132 < stable_array_2d[stable_array[stable_variable]][stable_array[stable_variable]];
        i_132++
    )
        {volatile int keep = 1; if (keep) break;}
    for (int i_133 = 0; i_133 < stable_array_2d[stable_array[_size_t]][stable_array[_size_t]]; i_133++)
        {volatile int keep = 1; if (keep) break;}
    for (
        int i_134 = 0;
        i_134 < stable_array_2d[stable_array[stable_array[0]]][stable_array[stable_array[0]]];
        i_134++
    )
        {volatile int keep = 1; if (keep) break;}
    for (
        int i_135 = 0;
        i_135 < stable_array_2d[stable_array[stable_outer_struct.x]][stable_array[stable_outer_struct.x]];
        i_135++
    )
        {volatile int keep = 1; if (keep) break;}
    for (int i_136 = 0; i_136 < stable_array_2d[stable_outer_struct.x][stable_outer_struct.x]; i_136++)
        {volatile int keep = 1; if (keep) break;}
    int stable_array_temp_0[10] = {0};
    for (int i_137 = 0; i_137 < stable_array_temp_0[0]; i_137++) {volatile int keep = 1; if (keep) break;}
    int* stable_array_temp_0_ptr_0 = stable_array_temp_0;
    int* stable_array_temp_0_ptr_1 = &stable_array_temp_0[1];
    int (*stable_array_temp_0_ptr_2)[10] = &stable_array_temp_0;

    for (int i_138 = 0; i_138 < stable_outer_struct.x; i_138++) {volatile int keep = 1; if (keep) break;}
    for (int i_139 = 0; i_139 < stable_outer_struct.inner_struct.x; i_139++) {volatile int keep = 1; if (keep) break;}
    for (int i_140 = 0; i_140 < stable_outer_struct_const.x; i_140++) {volatile int keep = 1; if (keep) break;}
    for (int i_141 = 0; i_141 < stable_outer_struct_const.inner_struct.x; i_141++)
        {volatile int keep = 1; if (keep) break;}
    outer_struct_t stable_outer_struct_temp_0 = { 0, { 0 } };
    for (int i_142 = 0; i_142 < stable_outer_struct_temp_0.x; i_142++) {volatile int keep = 1; if (keep) break;}
    for (int i_143 = 0; i_143 < stable_outer_struct_temp_0.inner_struct.x; i_143++)
        {volatile int keep = 1; if (keep) break;}
    int* stable_outer_struct_temp_0_ptr_0 = &stable_outer_struct_temp_0.inner_struct.x;
    outer_struct_t* stable_outer_struct_temp_0_ptr_1 = &stable_outer_struct_temp_0;

    for (int i_144 = 0; i_144 < type_alias_value; i_144++) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_145 = 0; i_145 < type_alias_value; i_145++) {volatile int keep = 1; if (keep) break;}
    for (long long i_146 = 0; i_146 < type_alias_value; i_146++) {volatile int keep = 1; if (keep) break;}
    for (unsigned long long i_147 = 0; i_147 < type_alias_value; i_147++) {volatile int keep = 1; if (keep) break;}
    for (int32_t i_148 = 0; i_148 < type_alias_value; i_148++) {volatile int keep = 1; if (keep) break;}
    for (uint32_t i_149 = 0; i_149 < type_alias_value; i_149++) {volatile int keep = 1; if (keep) break;}
    for (int64_t i_150 = 0; i_150 < type_alias_value; i_150++) {volatile int keep = 1; if (keep) break;}
    for (uint64_t i_151 = 0; i_151 < type_alias_value; i_151++) {volatile int keep = 1; if (keep) break;}
    for (intptr_t i_152 = 0; i_152 < type_alias_value; i_152++) {volatile int keep = 1; if (keep) break;}
    for (uintptr_t i_153 = 0; i_153 < type_alias_value; i_153++) {volatile int keep = 1; if (keep) break;}
    for (size_t i_154 = 0; i_154 < type_alias_value; i_154++) {volatile int keep = 1; if (keep) break;}

    for (int32_t i_155 = 0; i_155 < INT16_MAX; i_155++) {volatile int keep = 1; if (keep) break;}
    for (int32_t i_156 = 0; INT16_MAX > i_156; i_156++) {volatile int keep = 1; if (keep) break;}

    receive_stable_t_values_for_n
    (
        stable_variable,
        stable_variable_const,
        stable_outer_struct,
        stable_outer_struct_const
    );

    for (int i_157 = 0; i_157 < _signed_char; i_157++) {volatile int keep = 1; if (keep) break;}
    for (int i_158 = 0; i_158 < _unsigned_char; i_158++) {volatile int keep = 1; if (keep) break;}
    for (int i_159 = 0; i_159 < _short; i_159++) {volatile int keep = 1; if (keep) break;}
    for (int i_160 = 0; i_160 < _unsigned_short; i_160++) {volatile int keep = 1; if (keep) break;}
    for (int i_161 = 0; i_161 < _int8_t; i_161++) {volatile int keep = 1; if (keep) break;}
    for (int i_162 = 0; i_162 < _uint8_t; i_162++) {volatile int keep = 1; if (keep) break;}
    for (int i_163 = 0; i_163 < _int16_t; i_163++) {volatile int keep = 1; if (keep) break;}
    for (int i_164 = 0; i_164 < _uint16_t; i_164++) {volatile int keep = 1; if (keep) break;}

    stable_array[0] = 0;
    for (int i_165 = 0; i_165 > 10; i_165--) {volatile int keep = 1; if (keep) break;}
    for (int i_166 = 0; VARIABLE_GLOBAL_CONST < i_166; i_166--) {volatile int keep = 1; if (keep) break;}
    for (int i_167 = 0; i_167 > stable_variable; i_167--) {volatile int keep = 1; if (keep) break;}
    for (int i_168 = 0; i_168 > stable_variable_const; i_168--) {volatile int keep = 1; if (keep) break;}
    for (int i_169 = 0; stable_array[0] < i_169; i_169--) {volatile int keep = 1; if (keep) break;}
    for (int i_170 = 0; stable_array_const[0] < i_170; i_170--) {volatile int keep = 1; if (keep) break;}
    for (int i_171 = 0; stable_array_2d[0][0] < i_171; i_171--) {volatile int keep = 1; if (keep) break;}
    for (int i_172 = 0; i_172 > stable_outer_struct.x; i_172--) {volatile int keep = 1; if (keep) break;}
    for (int i_173 = 0; i_173 > stable_outer_struct_const.inner_struct.x; i_173--)
        {volatile int keep = 1; if (keep) break;}
    for (int i_174 = 0; type_alias_value < i_174; i_174--) {volatile int keep = 1; if (keep) break;}
    for (int i_175 = 0; _signed_char < i_175; i_175--) {volatile int keep = 1; if (keep) break;}
    for (int i_176 = 0; _unsigned_char < i_176; i_176--) {volatile int keep = 1; if (keep) break;}
    for (int i_177 = 0; _short < i_177; i_177--) {volatile int keep = 1; if (keep) break;}
    for (int i_178 = 0; _unsigned_short < i_178; i_178--) {volatile int keep = 1; if (keep) break;}
    for (int i_179 = 0; _int8_t < i_179; i_179--) {volatile int keep = 1; if (keep) break;}
    for (int i_180 = 0; _uint8_t < i_180; i_180--) {volatile int keep = 1; if (keep) break;}
    for (int i_181 = 0; _int16_t < i_181; i_181--) {volatile int keep = 1; if (keep) break;}
    for (int i_182 = 0; _uint16_t < i_182; i_182--) {volatile int keep = 1; if (keep) break;}

    receive_statically_determinable_t_values_for_n
    (
        stable_variable,
        stable_outer_struct
    );

    for (int f_4 = 0; f_4 < variable; f_4++) {volatile int keep = 1; if (keep) break;}
    static int variable_static_temp = 0;
    for (int f_5 = 0; f_5 < variable_static_temp; f_5++) {volatile int keep = 1; if (keep) break;}
    int variable_temp_0 = 0;
    for (int f_6 = 0; f_6 < variable_temp_0; f_6++)
    {
        int* variable_temp_0_ptr = &variable_temp_0;
        volatile int keep = 1;
        if (keep) break;
    }
    for (int f_7 = 0; f_7 < stable_variable; f_7++) {stable_variable = 0; volatile int keep = 1; if (keep) break;}

    for (int f_8 = 0; f_8 < array_static[0]; f_8++) {volatile int keep = 1; if (keep) break;}
    variable = 0;
    size_t _size_t_temp = 0;
    size_t* _size_t_temp_ptr = &_size_t_temp;
    stable_array[0] = 0;
    outer_struct.x = 0;
    for (int f_9 = 0; f_9 < stable_array[variable]; f_9++) {volatile int keep = 1; if (keep) break;}
    for (int f_10 = 0; f_10 < stable_array[_size_t_temp]; f_10++) {volatile int keep = 1; if (keep) break;}
    for (int f_11 = 0; f_11 < stable_array[stable_array[variable]]; f_11++) {volatile int keep = 1; if (keep) break;}
    for (int f_12 = 0; f_12 < stable_array[stable_array[_size_t_temp]]; f_12++)
        {volatile int keep = 1; if (keep) break;}
    for (int f_13 = 0; f_13 < stable_array[stable_array[stable_array[variable]]]; f_13++)
        {volatile int keep = 1; if (keep) break;}
    for (int f_14 = 0; f_14 < stable_array[stable_array[outer_struct.x]]; f_14++)
        {volatile int keep = 1; if (keep) break;}
    for (int f_15 = 0; f_15 < stable_array[outer_struct.x]; f_15++) {volatile int keep = 1; if (keep) break;}
    int array_temp_0[10] = {0};
    int array_temp_1[10] = {0};
    int array_temp_2[10] = {0};
    int* array_temp_0_ptr = array_temp_0;
    int* array_temp_1_ptr = &array_temp_1[1];
    int (*array_temp_2_ptr)[10] = &array_temp_2;
    for (int f_16 = 0; f_16 < array_temp_0[0]; f_16++) {volatile int keep = 1; if (keep) break;}
    for (int f_17 = 0; f_17 < array_temp_1[0]; f_17++) {volatile int keep = 1; if (keep) break;}
    for (int f_18 = 0; f_18 < array_temp_2[0]; f_18++) {volatile int keep = 1; if (keep) break;}
    int array_temp_3[10] = {0};
    int array_temp_4[10] = {0};
    int array_temp_5[10] = {0};
    for (int f_19 = 0; f_19 < array_temp_3[0]; f_19++)
    {
        int* array_temp_3_ptr = array_temp_3;
        volatile int keep = 1;
        if (keep) break;
    }
    for (int f_20 = 0; f_20 < array_temp_4[0]; f_20++)
    {
        int* array_temp_4_ptr = &array_temp_4[1];
        volatile int keep = 1;
        if (keep) break;
    }
    for (int f_21 = 0; f_21 < array_temp_5[0]; f_21++)
    {
        int (*array_temp_5_ptr)[10] = &array_temp_5;
        volatile int keep = 1;
        if (keep) break;
    }
    for (int f_22 = 0; f_22 < stable_array[0]; f_22++) {stable_array[1] = 0; volatile int keep = 1; if (keep) break;}

    for (int f_23 = 0; f_23 < outer_struct.x; f_23++) {volatile int keep = 1; if (keep) break;}
    static outer_struct_t outer_struct_static_temp = { 0, { 0 } };
    for (int f_24 = 0; f_24 < outer_struct_static_temp.inner_struct.x; f_24++) {volatile int keep = 1; if (keep) break;}
    outer_struct_t outer_struct_temp_0 = { 0, { 0 } };
    int* outer_struct_temp_0_ptr = &outer_struct_temp_0.inner_struct.x;
    for (int f_25 = 0; f_25 < outer_struct_temp_0.x; f_25++) {volatile int keep = 1; if (keep) break;}
    outer_struct_t outer_struct_temp_1 = { 0, { 0 } };
    outer_struct_t outer_struct_temp_2 = { 0, { 0 } };
    for (int f_26 = 0; f_26 < outer_struct_temp_1.x; f_26++)
    {
        int* outer_struct_temp_1_ptr = &outer_struct_temp_1.inner_struct.x;
        volatile int keep = 1;
        if (keep) break;
    }
    for (int f_27 = 0; f_27 < outer_struct_temp_2.inner_struct.x; f_27++)
    {
        outer_struct_t* outer_struct_temp_2_ptr = &outer_struct_temp_2;
        volatile int keep = 1;
        if (keep) break;
    }
    outer_struct_t outer_struct_temp_3 = { 0, { 0 } };
    outer_struct_t outer_struct_temp_4 = { 0, { 0 } };
    outer_struct_t outer_struct_temp_5 = { 0, { 0 } };
    for (int f_28 = 0; f_28 < outer_struct_temp_3.x; f_28++)
    {
        outer_struct_temp_3 = (outer_struct_t){ 0, { 0 } };
        volatile int keep = 1;
        if (keep) break;
    }
    for (int f_29 = 0; f_29 < outer_struct_temp_4.x; f_29++)
    {
        outer_struct_temp_4.inner_struct.x = 0;
        volatile int keep = 1;
        if (keep) break;
    }
    for (int f_30 = 0; f_30 < outer_struct_temp_5.inner_struct.x; f_30++)
    {
        outer_struct_temp_5.x = 0;
        volatile int keep = 1;
        if (keep) break;
    }

    stable_variable = stable_variable_const;
    for (int f_31 = 0; f_31 > stable_variable; f_31--) {volatile int keep = 1; if (keep) break;}
    stable_variable = 0;
    stable_array[0] = stable_variable_const;
    for (int f_32 = 0; stable_array[0] < f_32; f_32--) {volatile int keep = 1; if (keep) break;}
    stable_array[0] = 0;
    for (int f_33 = 0; f_33 > stable_array[stable_variable_const]; f_33--) {volatile int keep = 1; if (keep) break;}
    stable_outer_struct.x = stable_variable_const;
    for (int f_34 = 0; stable_outer_struct.x < f_34; f_34--) {volatile int keep = 1; if (keep) break;}
    stable_outer_struct.x = 0;
    for (uint32_t f_35 = 0; f_35 > UINT32_C(4294967295); f_35--) {volatile int keep = 1; if (keep) break;}
    for (int64_t f_36 = 0; INT64_MAX < f_36; f_36--) {volatile int keep = 1; if (keep) break;}
    const uint64_t uint64_max = UINT64_C(18446744073709551615);
    for (uint64_t f_37 = 0; f_37 > uint64_max; f_37--) {volatile int keep = 1; if (keep) break;}

    // decrementing_or_incrementing

    for (int i_183 = 0; i_183 < 10; i_183++) {volatile int keep = 1; if (keep) break;}
    for (int i_184 = 0; i_184 <= 10; i_184++) {volatile int keep = 1; if (keep) break;}
    for (int i_185 = 0; 10 > i_185; i_185++) {volatile int keep = 1; if (keep) break;}
    for (int i_186 = 0; 10 >= i_186; i_186++) {volatile int keep = 1; if (keep) break;}

    for (int f_38 = 0; 10 <= f_38; f_38++) {volatile int keep = 1; if (keep) break;}

    for (int i_187 = 0; i_187 > 10; i_187--) {volatile int keep = 1; if (keep) break;}
    for (int i_188 = 0; i_188 >= 10; i_188--) {volatile int keep = 1; if (keep) break;}
    for (int i_189 = 0; 10 < i_189 ; i_189--) {volatile int keep = 1; if (keep) break;}
    for (int i_190 = 0; 10 <= i_190; i_190--) {volatile int keep = 1; if (keep) break;}

    for (int f_39 = 0; 10 >= f_39; f_39--) {volatile int keep = 1; if (keep) break;}

    for (int i_191 = 0; i_191 < 10; i_191 += 1) {volatile int keep = 1; if (keep) break;}
    for (int i_192 = 0; i_192 <= 10; i_192 += 1) {volatile int keep = 1; if (keep) break;}
    for (int i_193 = 0; 10 > i_193; i_193 += 2) {volatile int keep = 1; if (keep) break;}
    for (int i_194 = 0; 10 >= i_194; i_194 += 2) {volatile int keep = 1; if (keep) break;}

    for (int f_40 = 0; 10 < f_40; f_40 += 1) {volatile int keep = 1; if (keep) break;}

    for (int i_195 = 0; i_195 > 10; i_195 -= 1) {volatile int keep = 1; if (keep) break;}
    for (int i_196 = 0; i_196 >= 10; i_196 -= 1) {volatile int keep = 1; if (keep) break;}
    for (int i_197 = 0; 10 < i_197; i_197 -= 2) {volatile int keep = 1; if (keep) break;}
    for (int i_198 = 0; 10 <= i_198; i_198 -= 2) {volatile int keep = 1; if (keep) break;}

    for (int f_41 = 0; 10 > f_41; f_41 -= 1) {volatile int keep = 1; if (keep) break;}

    for (int i_199 = 0; i_199 > 10; i_199 += -1) {volatile int keep = 1; if (keep) break;}
    for (int i_200 = 0; i_200 >= 10; i_200 += -1) {volatile int keep = 1; if (keep) break;}
    for (int i_201 = 0; 10 < i_201; i_201 += -2) {volatile int keep = 1; if (keep) break;}
    for (int i_202 = 0; 10 <= i_202; i_202 += -2) {volatile int keep = 1; if (keep) break;}

    for (int f_42 = 0; 10 >= f_42; f_42 += -2) {volatile int keep = 1; if (keep) break;}

    for (int i_203 = 0; i_203 < 10; i_203 -= -1) {volatile int keep = 1; if (keep) break;}
    for (int i_204 = 0; i_204 <= 10; i_204 -= -1) {volatile int keep = 1; if (keep) break;}
    for (int i_205 = 0; 10 > i_205; i_205 -= -2) {volatile int keep = 1; if (keep) break;}
    for (int i_206 = 0; 10 >= i_206; i_206 -= -2) {volatile int keep = 1; if (keep) break;}

    for (int f_43 = 0; 10 <= f_43; f_43 -= -2) {volatile int keep = 1; if (keep) break;}

    for (int i_207 = 0; i_207 < 10; i_207 = i_207 + 1) {volatile int keep = 1; if (keep) break;}
    for (int i_208 = 0; i_208 <= 10; i_208 = i_208 + 1) {volatile int keep = 1; if (keep) break;}
    for (int i_209 = 0; 10 > i_209; i_209 = i_209 + 2) {volatile int keep = 1; if (keep) break;}
    for (int i_210 = 0; 10 >= i_210; i_210 = i_210 + 2) {volatile int keep = 1; if (keep) break;}

    for (int f_44 = 0; 10 < f_44; f_44 = f_44 + 1) {volatile int keep = 1; if (keep) break;}

    for (int i_211 = 0; i_211 > 10; i_211 = i_211 - 1) {volatile int keep = 1; if (keep) break;}
    for (int i_212 = 0; i_212 >= 10; i_212 = i_212 - 1) {volatile int keep = 1; if (keep) break;}
    for (int i_213 = 0; 10 < i_213; i_213 = i_213 - 2) {volatile int keep = 1; if (keep) break;}
    for (int i_214 = 0; 10 <= i_214; i_214 = i_214 - 2) {volatile int keep = 1; if (keep) break;}

    for (int f_45 = 0; 10 > f_45; f_45 = f_45 - 1) {volatile int keep = 1; if (keep) break;}

    for (int i_215 = 0; i_215 > 10; i_215 = i_215 + -1) {volatile int keep = 1; if (keep) break;}
    for (int i_216 = 0; i_216 >= 10; i_216 = i_216 + -1) {volatile int keep = 1; if (keep) break;}
    for (int i_217 = 0; 10 < i_217; i_217 = i_217 + -2) {volatile int keep = 1; if (keep) break;}
    for (int i_218 = 0; 10 <= i_218; i_218 = i_218 + -2) {volatile int keep = 1; if (keep) break;}

    for (int f_46 = 0; 10 >= f_46; f_46 = f_46 + -2) {volatile int keep = 1; if (keep) break;}

    for (int i_219 = 0; i_219 < 10; i_219 = i_219 - -1) {volatile int keep = 1; if (keep) break;}
    for (int i_220 = 0; i_220 <= 10; i_220 = i_220 - -1) {volatile int keep = 1; if (keep) break;}
    for (int i_221 = 0; 10 > i_221; i_221 = i_221 - -2) {volatile int keep = 1; if (keep) break;}
    for (int i_222 = 0; 10 >= i_222; i_222 = i_222 - -2) {volatile int keep = 1; if (keep) break;}

    for (int f_47 = 0; 10 <= f_47; f_47 = f_47 - -2) {volatile int keep = 1; if (keep) break;}

    for (int i_223 = 0; i_223 < 10; i_223 = 1 + i_223) {volatile int keep = 1; if (keep) break;}
    for (int i_224 = 0; i_224 <= 10; i_224 = 1 + i_224) {volatile int keep = 1; if (keep) break;}
    for (int i_225 = 0; 10 > i_225; i_225 = 2 + i_225) {volatile int keep = 1; if (keep) break;}
    for (int i_226 = 0; 10 >= i_226; i_226 = 2 + i_226) {volatile int keep = 1; if (keep) break;}

    for (int f_48 = 0; 10 < f_48; f_48 = 1 + f_48) {volatile int keep = 1; if (keep) break;}

    for (int i_227 = 0; i_227 > 10; i_227 = -1 + i_227) {volatile int keep = 1; if (keep) break;}
    for (int i_228 = 0; i_228 >= 10; i_228 = -1 + i_228) {volatile int keep = 1; if (keep) break;}
    for (int i_229 = 0; 10 < i_229; i_229 = -2 + i_229) {volatile int keep = 1; if (keep) break;}
    for (int i_230 = 0; 10 <= i_230; i_230 = -2 + i_230) {volatile int keep = 1; if (keep) break;}

    for (int f_49 = 0; 10 > f_49; f_49 = -1 + f_49) {volatile int keep = 1; if (keep) break;}

    for (unsigned int i_231 = 0; i_231 < 10; i_231++) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_232 = 0; i_232 <= 10; i_232++) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_233 = 0; 10 > i_233; i_233++) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_234 = 0; 10 >= i_234; i_234++) {volatile int keep = 1; if (keep) break;}

    for (unsigned int f_50 = 0; 10 <= f_50; f_50++) {volatile int keep = 1; if (keep) break;}

    for (unsigned int i_235 = 0; i_235 > 10; i_235--) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_236 = 0; i_236 >= 10; i_236--) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_237 = 0; 10 < i_237 ; i_237--) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_238 = 0; 10 <= i_238; i_238--) {volatile int keep = 1; if (keep) break;}

    for (unsigned int f_51 = 0; 10 >= f_51; f_51--) {volatile int keep = 1; if (keep) break;}

    for (unsigned int i_239 = 0; i_239 < 10; i_239 += 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_240 = 0; i_240 <= 10; i_240 += 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_241 = 0; 10 > i_241; i_241 += 2) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_242 = 0; 10 >= i_242; i_242 += 2) {volatile int keep = 1; if (keep) break;}

    for (unsigned int f_52 = 0; 10 < f_52; f_52 += 1) {volatile int keep = 1; if (keep) break;}

    for (unsigned int i_243 = 0; i_243 > 10; i_243 -= 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_244 = 0; i_244 >= 10; i_244 -= 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_245 = 0; 10 < i_245; i_245 -= 2) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_246 = 0; 10 <= i_246; i_246 -= 2) {volatile int keep = 1; if (keep) break;}

    for (unsigned int f_53 = 0; 10 > f_53; f_53 -= 1) {volatile int keep = 1; if (keep) break;}

    for (unsigned int i_247 = 0; i_247 < 10; i_247 = i_247 + 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_248 = 0; i_248 <= 10; i_248 = i_248 + 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_249 = 0; 10 > i_249; i_249 = i_249 + 2) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_250 = 0; 10 >= i_250; i_250 = i_250 + 2) {volatile int keep = 1; if (keep) break;}

    for (unsigned int f_54 = 0; 10 <= f_54; f_54 = f_54 + 2) {volatile int keep = 1; if (keep) break;}

    for (unsigned int i_251 = 0; i_251 > 10; i_251 = i_251 - 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_252 = 0; i_252 >= 10; i_252 = i_252 - 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_253 = 0; 10 < i_253; i_253 = i_253 - 2) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_254 = 0; 10 <= i_254; i_254 = i_254 - 2) {volatile int keep = 1; if (keep) break;}

    for (unsigned int f_55 = 0; 10 >= f_55; f_55 = f_55 - 2) {volatile int keep = 1; if (keep) break;}

    for (unsigned int i_255 = 0; i_255 < 10; i_255 = 1 + i_255) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_256 = 0; i_256 <= 10; i_256 = 1 + i_256) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_257 = 0; 10 > i_257; i_257 = 2 + i_257) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_258 = 0; 10 >= i_258; i_258 = 2 + i_258) {volatile int keep = 1; if (keep) break;}

    for (unsigned int f_56 = 0; 10 < f_56; f_56 = 1 + f_56) {volatile int keep = 1; if (keep) break;}

    _signed_char = 1;
    for (int i_259 = 0; i_259 < 10; i_259 += _signed_char) {volatile int keep = 1; if (keep) break;}
    for (int i_260 = 0; i_260 <= 10; i_260 += _signed_char) {volatile int keep = 1; if (keep) break;}
    for (int i_261 = 0; 10 > i_261; i_261 += _signed_char) {volatile int keep = 1; if (keep) break;}
    for (int i_262 = 0; 10 >= i_262; i_262 += _signed_char) {volatile int keep = 1; if (keep) break;}

    for (int f_57 = 0; 10 < f_57; f_57 += _signed_char) {volatile int keep = 1; if (keep) break;}

    for (int i_263 = 0; i_263 > 10; i_263 -= _signed_char) {volatile int keep = 1; if (keep) break;}
    for (int i_264 = 0; i_264 >= 10; i_264 -= _signed_char) {volatile int keep = 1; if (keep) break;}
    for (int i_265 = 0; 10 < i_265; i_265 -= _signed_char) {volatile int keep = 1; if (keep) break;}
    for (int i_266 = 0; 10 <= i_266; i_266 -= _signed_char) {volatile int keep = 1; if (keep) break;}

    for (int f_58 = 0; 10 >= f_58; f_58 -= _signed_char) {volatile int keep = 1; if (keep) break;}

    _short = -1;
    for (int i_267 = 0; i_267 > 10; i_267 += _short) {volatile int keep = 1; if (keep) break;}
    for (int i_268 = 0; i_268 >= 10; i_268 += _short) {volatile int keep = 1; if (keep) break;}
    for (int i_269 = 0; 10 < i_269; i_269 += _short) {volatile int keep = 1; if (keep) break;}
    for (int i_270 = 0; 10 <= i_270; i_270 += _short) {volatile int keep = 1; if (keep) break;}

    for (int f_59 = 0; 10 >= f_59; f_59 += _short) {volatile int keep = 1; if (keep) break;}

    for (int i_271 = 0; i_271 < 10; i_271 -= _short) {volatile int keep = 1; if (keep) break;}
    for (int i_272 = 0; i_272 <= 10; i_272 -= _short) {volatile int keep = 1; if (keep) break;}
    for (int i_273 = 0; 10 > i_273; i_273 -= _short) {volatile int keep = 1; if (keep) break;}
    for (int i_274 = 0; 10 >= i_274; i_274 -= _short) {volatile int keep = 1; if (keep) break;}

    for (int f_60 = 0; 10 < f_60; f_60 -= _short) {volatile int keep = 1; if (keep) break;}

    stable_variable = 1;
    for (int i_275 = 0; i_275 < 10; i_275 = i_275 + stable_variable) {volatile int keep = 1; if (keep) break;}
    for (int i_276 = 0; i_276 <= 10; i_276 = i_276 + stable_variable) {volatile int keep = 1; if (keep) break;}
    for (int i_277 = 0; 10 > i_277; i_277 = i_277 + stable_variable) {volatile int keep = 1; if (keep) break;}
    for (int i_278 = 0; 10 >= i_278; i_278 = i_278 + stable_variable) {volatile int keep = 1; if (keep) break;}

    for (int f_61 = 0; 10 < f_61; f_61 = f_61 + stable_variable) {volatile int keep = 1; if (keep) break;}
    variable = 1;
    for (int f_62 = 0; 10 >= f_62; f_62 = f_62 + variable) {volatile int keep = 1; if (keep) break;}

    for (int i_279 = 0; i_279 > 10; i_279 = i_279 - stable_variable) {volatile int keep = 1; if (keep) break;}
    for (int i_280 = 0; i_280 >= 10; i_280 = i_280 - stable_variable) {volatile int keep = 1; if (keep) break;}
    for (int i_281 = 0; 10 < i_281; i_281 = i_281 - stable_variable) {volatile int keep = 1; if (keep) break;}
    for (int i_282 = 0; 10 <= i_282; i_282 = i_282 - stable_variable) {volatile int keep = 1; if (keep) break;}

    for (int f_63 = 0; 10 >= f_63; f_63 = f_63 - stable_variable) {volatile int keep = 1; if (keep) break;}

    _int8_t = -2;
    for (int i_283 = 0; i_283 > 10; i_283 = i_283 + _int8_t) {volatile int keep = 1; if (keep) break;}
    for (int i_284 = 0; i_284 >= 10; i_284 = i_284 + _int8_t) {volatile int keep = 1; if (keep) break;}
    for (int i_285 = 0; 10 < i_285; i_285 = i_285 + _int8_t) {volatile int keep = 1; if (keep) break;}
    for (int i_286 = 0; 10 <= i_286; i_286 = i_286 + _int8_t) {volatile int keep = 1; if (keep) break;}

    for (int f_64 = 0; 10 >= f_64; f_64 = f_64 + _int8_t) {volatile int keep = 1; if (keep) break;}

    for (int i_287 = 0; i_287 < 10; i_287 = i_287 - _int8_t) {volatile int keep = 1; if (keep) break;}
    for (int i_288 = 0; i_288 <= 10; i_288 = i_288 - _int8_t) {volatile int keep = 1; if (keep) break;}
    for (int i_289 = 0; 10 > i_289; i_289 = i_289 - _int8_t) {volatile int keep = 1; if (keep) break;}
    for (int i_290 = 0; 10 >= i_290; i_290 = i_290 - _int8_t) {volatile int keep = 1; if (keep) break;}

    for (int f_65 = 0; 10 < f_65; f_65 = f_65 - _int8_t) {volatile int keep = 1; if (keep) break;}

    _int16_t = 2;
    for (int i_291 = 0; i_291 < 10; i_291 = _int16_t + i_291) {volatile int keep = 1; if (keep) break;}
    for (int i_292 = 0; i_292 <= 10; i_292 = _int16_t + i_292) {volatile int keep = 1; if (keep) break;}
    for (int i_293 = 0; 10 > i_293; i_293 = _int16_t + i_293) {volatile int keep = 1; if (keep) break;}
    for (int i_294 = 0; 10 >= i_294; i_294 = _int16_t + i_294) {volatile int keep = 1; if (keep) break;}

    for (int f_66 = 0; 10 < f_66; f_66 = _int16_t + f_66) {volatile int keep = 1; if (keep) break;}

    _int16_t = -2;
    for (int i_295 = 0; i_295 > 10; i_295 = _int16_t + i_295) {volatile int keep = 1; if (keep) break;}
    for (int i_296 = 0; i_296 >= 10; i_296 = _int16_t + i_296) {volatile int keep = 1; if (keep) break;}
    for (int i_297 = 0; 10 < i_297; i_297 = _int16_t + i_297) {volatile int keep = 1; if (keep) break;}
    for (int i_298 = 0; 10 <= i_298; i_298 = _int16_t + i_298) {volatile int keep = 1; if (keep) break;}

    for (int f_67 = 0; 10 >= f_67; f_67 = _int16_t + f_67) {volatile int keep = 1; if (keep) break;}

    #define VARIABLE_GLOBAL_TEMP 1
    const int stable_variable_const_temp = -2;
    stable_array[0] = 3;
    const int stable_array_const_temp[10] = {-4};
    stable_array_2d[0][0] = 5;
    stable_outer_struct.x = -6;
    const outer_struct_t stable_outer_struct_const_temp = { 0, { 7 } };
    enum { type_alias_value_temp = -8 };
    for (int i_299 = 0; i_299 < 10; i_299 += VARIABLE_GLOBAL_TEMP) {volatile int keep = 1; if (keep) break;}
    for (int i_300 = 0; i_300 < 10; i_300 -= stable_variable_const_temp) {volatile int keep = 1; if (keep) break;}
    for (int i_301 = 0; i_301 < 10; i_301 = i_301 +  stable_array[0]) {volatile int keep = 1; if (keep) break;}
    for (int i_302 = 0; i_302 < 10; i_302 = i_302 -  stable_array_const_temp[0]) {volatile int keep = 1; if (keep) break;}
    for (int i_303 = 0; i_303 > 10; i_303 -= stable_array_2d[0][0]) {volatile int keep = 1; if (keep) break;}
    for (int i_304 = 0; i_304 > 10; i_304 += stable_outer_struct.x) {volatile int keep = 1; if (keep) break;}
    for (int i_305 = 0; i_305 > 10; i_305 = i_305 - stable_outer_struct_const_temp.inner_struct.x)
        {volatile int keep = 1; if (keep) break;}
    for (int i_306 = 0; i_306 > 10; i_306 = i_306 + type_alias_value_temp) {volatile int keep = 1; if (keep) break;}

    stable_variable = stable_variable_const;
    for (int f_68 = 0; f_68 < 10; f_68 += stable_variable) {volatile int keep = 1; if (keep) break;}
    stable_variable = 0;
    stable_array[0] = stable_variable_const;
    for (int f_69 = 0; f_69 < 10; f_69 += stable_array[0]) {volatile int keep = 1; if (keep) break;}
    stable_array[0] = 0;
    for (int f_70 = 0; f_70 < 10; f_70 += stable_array[stable_variable_const]) {volatile int keep = 1; if (keep) break;}
    stable_outer_struct.x = stable_variable_const;
    for (int f_71 = 0; f_71 < 10; f_71 += stable_outer_struct.x) {volatile int keep = 1; if (keep) break;}
    stable_outer_struct.x = 0;
    for (int f_72 = 0; f_72 < 10; f_72 += VARIABLE_GLOBAL_CONST) {volatile int keep = 1; if (keep) break;}
    for (int f_73 = 0; f_73 < 10; f_73 += stable_variable_const) {volatile int keep = 1; if (keep) break;}
    for (int f_74 = 0; f_74 < 10; f_74 += stable_array_const[0]) {volatile int keep = 1; if (keep) break;}
    for (int f_75 = 0; f_75 < 10; f_75 += stable_outer_struct_const.x) {volatile int keep = 1; if (keep) break;}
    for (int f_76 = 0; f_76 < 10; f_76 += stable_outer_struct_const.inner_struct.x)
        {volatile int keep = 1; if (keep) break;}
    for (int f_77 = 0; f_77 < 10; f_77 += type_alias_value) {volatile int keep = 1; if (keep) break;}

    // body

    for (int f_78 = 0; f_78 < 10; f_78++) {f_78++; volatile int keep = 1; if (keep) break;}

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
    for (int i_307 = variable; i_307 < 10; i_307++) {volatile int keep = 1; if (keep) break;}
    for (int i_308 = variable_const; i_308 < 10; i_308++) {volatile int keep = 1; if (keep) break;}
    for (int i_309 = *pointer; i_309 < 10; i_309++) {volatile int keep = 1; if (keep) break;}
    for (int i_310 = *pointer_const_0; i_310 < 10; i_310++) {volatile int keep = 1; if (keep) break;}
    for (int i_311 = *pointer_const_1; i_311 < 10; i_311++) {volatile int keep = 1; if (keep) break;}
    for (int i_312 = **pointer_2d; i_312 < 10; i_312++) {volatile int keep = 1; if (keep) break;}
    for (int i_313 = outer_struct.x; i_313 < 10; i_313++) {volatile int keep = 1; if (keep) break;}
    for (int i_314 = outer_struct.inner_struct.x; i_314 < 10; i_314++) {volatile int keep = 1; if (keep) break;}
    for (int i_315 = outer_struct_const.x; i_315 < 10; i_315++) {volatile int keep = 1; if (keep) break;}
    for (int i_316 = outer_struct_const.inner_struct.x; i_316 < 10; i_316++) {volatile int keep = 1; if (keep) break;}
    for (int i_317 = outer_struct_ptr->x; i_317 < 10; i_317++) {volatile int keep = 1; if (keep) break;}
    for (int i_318 = outer_struct_ptr->inner_struct.x; i_318 < 10; i_318++) {volatile int keep = 1; if (keep) break;}
    for (int i_319 = outer_struct_ptr_const_0->x; i_319 < 10; i_319++) {volatile int keep = 1; if (keep) break;}
    for (int i_320 = outer_struct_ptr_const_0->inner_struct.x; i_320 < 10; i_320++)
        {volatile int keep = 1; if (keep) break;}
    for (int i_321 = outer_struct_ptr_const_1->x; i_321 < 10; i_321++) {volatile int keep = 1; if (keep) break;}
    for (int i_322 = outer_struct_ptr_const_1->inner_struct.x; i_322 < 10; i_322++)
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
    for (int i_323 = 0; i_323 < stable_variable; i_323++) {volatile int keep = 1; if (keep) break;}
    for (int i_324 = 0; i_324 < stable_variable_const; i_324++) {volatile int keep = 1; if (keep) break;}
    for (int i_325 = 0; i_325 < stable_outer_struct.x; i_325++) {volatile int keep = 1; if (keep) break;}
    for (int i_326 = 0; i_326 < stable_outer_struct.inner_struct.x; i_326++) {volatile int keep = 1; if (keep) break;}
    for (int i_327 = 0; i_327 < stable_outer_struct_const.x; i_327++) {volatile int keep = 1; if (keep) break;}
    for (int i_328 = 0; i_328 < stable_outer_struct_const.inner_struct.x; i_328++)
        {volatile int keep = 1; if (keep) break;}

    int* stable_variable_ptr = &stable_variable;
    for (int f_79 = 0; f_79 < stable_variable; f_79++) {volatile int keep = 1; if (keep) break;}
    outer_struct_t* stable_outer_struct_ptr = &stable_outer_struct;
    for (int f_80 = 0; f_80 < stable_outer_struct.x; f_80++) {volatile int keep = 1; if (keep) break;}
}

static void receive_statically_determinable_t_values_for_n
(
    int stable_variable,
    outer_struct_t stable_outer_struct
)
{
    for (int f_81 = 0; f_81 > stable_variable; f_81--) {volatile int keep = 1; if (keep) break;}
    for (int f_82 = 0; stable_outer_struct.x < f_82; f_82--) {volatile int keep = 1; if (keep) break;}
    for (int f_83 = 0; f_83 > stable_outer_struct.inner_struct.x; f_83--) {volatile int keep = 1; if (keep) break;}

    stable_variable = 0;
    stable_outer_struct.x = 0;
    stable_outer_struct.inner_struct.x = 0;
    for (int i_329 = 0; i_329  > stable_variable; i_329 --) {volatile int keep = 1; if (keep) break;}
    for (int i_330 = 0; stable_outer_struct.x < i_330; i_330 --) {volatile int keep = 1; if (keep) break;}
    for (int i_331 = 0; i_331 > stable_outer_struct.inner_struct.x; i_331 --)
        {volatile int keep = 1; if (keep) break;}
}
