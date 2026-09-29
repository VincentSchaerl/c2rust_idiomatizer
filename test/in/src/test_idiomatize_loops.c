#include <stdint.h>
#include <stdio.h>

#include "nested_struct.h"

static int return_int();

static unsigned char return_unsigned_char();

static signed long long return_signed_long_long();

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

extern int array_global[10];
static int array_global_static[10] = {0};
extern int array_2d_global[2][2];

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
    static int stable_array_static[10] = {0};
    int stable_array_2d[2][2] = {{0, 0}, {0, 0}};

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
    for (int i_26 = **pointer_2d_global; i_26 < 10; i_26++) {volatile int keep = 1; if (keep) break;}
    for (int i_27 = *pointer; i_27 < 10; i_27++) {volatile int keep = 1; if (keep) break;}
    for (int i_28 = *pointer_const_0; i_28 < 10; i_28++) {volatile int keep = 1; if (keep) break;}
    for (int i_29 = *pointer_const_1; i_29 < 10; i_29++) {volatile int keep = 1; if (keep) break;}
    for (int i_30 = *pointer_static; i_30 < 10; i_30++) {volatile int keep = 1; if (keep) break;}
    for (int i_31 = **pointer_2d; i_31 < 10; i_31++) {volatile int keep = 1; if (keep) break;}

    const int i = 0;
    for (int i_32 = array_global[0]; i_32 < 10; i_32++) {volatile int keep = 1; if (keep) break;}
    for (int i_33 = array_global[i]; i_33 < 10; i_33++) {volatile int keep = 1; if (keep) break;}
    for (int i_34 = array_global_static[0]; i_34 < 10; i_34++) {volatile int keep = 1; if (keep) break;}
    for (int i_35 = array_global_static[i]; i_35 < 10; i_35++) {volatile int keep = 1; if (keep) break;}
    for (int i_36 = array_2d_global[0][0]; i_36 < 10; i_36++) {volatile int keep = 1; if (keep) break;}
    for (int i_37 = array_2d_global[i][i]; i_37 < 10; i_37++) {volatile int keep = 1; if (keep) break;}
    for (int i_38 = array[0]; i_38 < 10; i_38++) {volatile int keep = 1; if (keep) break;}
    for (int i_39 = array[i]; i_39 < 10; i_39++) {volatile int keep = 1; if (keep) break;}
    for (int i_40 = array_const[0]; i_40 < 10; i_40++) {volatile int keep = 1; if (keep) break;}
    for (int i_41 = array_const[i]; i_41 < 10; i_41++) {volatile int keep = 1; if (keep) break;}
    for (int i_42 = array_static[0]; i_42 < 10; i_42++) {volatile int keep = 1; if (keep) break;}
    for (int i_43 = array_static[i]; i_43 < 10; i_43++) {volatile int keep = 1; if (keep) break;}
    for (int i_44 = array_2d[0][0]; i_44 < 10; i_44++) {volatile int keep = 1; if (keep) break;}
    for (int i_45 = array_2d[i][i]; i_45 < 10; i_45++) {volatile int keep = 1; if (keep) break;}

    for (int i_46 = outer_struct_global.x; i_46 < 10; i_46++) {volatile int keep = 1; if (keep) break;}
    for (int i_47 = outer_struct_global.inner_struct.x; i_47 < 10; i_47++) {volatile int keep = 1; if (keep) break;}
    for (int i_48 = outer_struct_global_static.x; i_48 < 10; i_48++) {volatile int keep = 1; if (keep) break;}
    for (int i_49 = outer_struct_global_static.inner_struct.x; i_49 < 10; i_49++) {volatile int keep = 1; if (keep) break;}
    for (int i_50 = outer_struct.x; i_50 < 10; i_50++) {volatile int keep = 1; if (keep) break;}
    for (int i_51 = outer_struct.inner_struct.x; i_51 < 10; i_51++) {volatile int keep = 1; if (keep) break;}
    for (int i_52 = outer_struct_const.x; i_52 < 10; i_52++) {volatile int keep = 1; if (keep) break;}
    for (int i_53 = outer_struct_const.inner_struct.x; i_53 < 10; i_53++) {volatile int keep = 1; if (keep) break;}
    for (int i_54 = outer_struct_static.x; i_54 < 10; i_54++) {volatile int keep = 1; if (keep) break;}
    for (int i_55 = outer_struct_static.inner_struct.x; i_55 < 10; i_55++) {volatile int keep = 1; if (keep) break;}

    for (int i_56 = outer_struct_ptr_global->x; i_56 < 10; i_56++) {volatile int keep = 1; if (keep) break;}
    for (int i_57 = outer_struct_ptr_global->inner_struct.x; i_57 < 10; i_57++) {volatile int keep = 1; if (keep) break;}
    for (int i_58 = outer_struct_ptr_global_const_0->x; i_58 < 10; i_58++) {volatile int keep = 1; if (keep) break;}
    for (int i_59 = outer_struct_ptr_global_const_0->inner_struct.x; i_59 < 10; i_59++) {volatile int keep = 1; if (keep) break;}
    for (int i_60 = outer_struct_ptr_global_static->x; i_60 < 10; i_60++) {volatile int keep = 1; if (keep) break;}
    for (int i_61 = outer_struct_ptr_global_static->inner_struct.x; i_61 < 10; i_61++) {volatile int keep = 1; if (keep) break;}
    for (int i_62 = outer_struct_ptr->x; i_62 < 10; i_62++) {volatile int keep = 1; if (keep) break;}
    for (int i_63 = outer_struct_ptr->inner_struct.x; i_63 < 10; i_63++) {volatile int keep = 1; if (keep) break;}
    for (int i_64 = outer_struct_ptr_const_0->x; i_64 < 10; i_64++) {volatile int keep = 1; if (keep) break;}
    for (int i_65 = outer_struct_ptr_const_0->inner_struct.x; i_65 < 10; i_65++) {volatile int keep = 1; if (keep) break;}
    for (int i_66 = outer_struct_ptr_const_1->x; i_66 < 10; i_66++) {volatile int keep = 1; if (keep) break;}
    for (int i_67 = outer_struct_ptr_const_1->inner_struct.x; i_67 < 10; i_67++) {volatile int keep = 1; if (keep) break;}
    for (int i_68 = outer_struct_ptr_static->x; i_68 < 10; i_68++) {volatile int keep = 1; if (keep) break;}
    for (int i_69 = outer_struct_ptr_static->inner_struct.x; i_69 < 10; i_69++) {volatile int keep = 1; if (keep) break;}

    for (int i_70 = return_int(); i_70 < 10; i_70++) {volatile int keep = 1; if (keep) break;}

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

    for (int i_71 = _signed_char; i_71 < 10; i_71++) {volatile int keep = 1; if (keep) break;}
    for (int i_72 = _unsigned_char; i_72 < 10; i_72++) {volatile int keep = 1; if (keep) break;}
    for (int i_73 = _short; i_73 < 10; i_73++) {volatile int keep = 1; if (keep) break;}
    for (int i_74 = _unsigned_short; i_74 < 10; i_74++) {volatile int keep = 1; if (keep) break;}
    for (int i_75 = _long_long; i_75 < 10; i_75++) {volatile int keep = 1; if (keep) break;}
    for (int i_76 = _unsigned_long_long; i_76 < 10; i_76++) {volatile int keep = 1; if (keep) break;}
    for (int i_77 = _int8_t; i_77 < 10; i_77++) {volatile int keep = 1; if (keep) break;}
    for (int i_78 = _uint8_t; i_78 < 10; i_78++) {volatile int keep = 1; if (keep) break;}
    for (int i_79 = _int16_t; i_79 < 10; i_79++) {volatile int keep = 1; if (keep) break;}
    for (int i_80 = _uint16_t; i_80 < 10; i_80++) {volatile int keep = 1; if (keep) break;}
    for (int i_81 = _uint32_t; i_81 < 10; i_81++) {volatile int keep = 1; if (keep) break;}
    for (int i_82 = _int64_t; i_82 < 10; i_82++) {volatile int keep = 1; if (keep) break;}
    for (int i_83 = _uint64_t; i_83 < 10; i_83++) {volatile int keep = 1; if (keep) break;}
    for (int i_84 = _intptr_t; i_84 < 10; i_84++) {volatile int keep = 1; if (keep) break;}
    for (int i_85 = _uintptr_t; i_85 < 10; i_85++) {volatile int keep = 1; if (keep) break;}
    for (int i_86 = _size_t; i_86 < 10; i_86++) {volatile int keep = 1; if (keep) break;}

    for (int i_87 = return_unsigned_char(); i_87 < 10; i_87++)  {volatile int keep = 1; if (keep) break;}
    for (int i_88 = return_signed_long_long(); i_88 < 10; i_88++)  {volatile int keep = 1; if (keep) break;}
    for (int i_89 = return_int8_t(); i_89 < 10; i_89++)  {volatile int keep = 1; if (keep) break;}
    for (int i_90 = return_uint64_t(); i_90 < 10; i_90++)  {volatile int keep = 1; if (keep) break;}
    for (int i_91 = return_intptr_t(); i_91 < 10; i_91++)  {volatile int keep = 1; if (keep) break;}
    for (int i_92 = return_uintptr_t(); i_92 < 10; i_92++)  {volatile int keep = 1; if (keep) break;}
    for (int i_93 = return_size_t(); i_93 < 10; i_93++)  {volatile int keep = 1; if (keep) break;}

    // relational_operator

    for (int i_94 = 0; i_94 < 10; i_94++) {volatile int keep = 1; if (keep) break;}
    for (int i_95 = 0; i_95 <= 10; i_95++) {volatile int keep = 1; if (keep) break;}
    for (int i_96 = 0; i_96 > 10; i_96--) {volatile int keep = 1; if (keep) break;}
    for (int i_97 = 0; i_97 >= 10; i_97--) {volatile int keep = 1; if (keep) break;}
    for (int i_98 = 0; 10 < i_98; i_98--) {volatile int keep = 1; if (keep) break;}
    for (int i_99 = 0; 10 <= i_99; i_99--) {volatile int keep = 1; if (keep) break;}
    for (int i_100 = 0; 10 > i_100; i_100++) {volatile int keep = 1; if (keep) break;}
    for (int i_101 = 0; 10 >= i_101; i_101++) {volatile int keep = 1; if (keep) break;}

    for (int f_3 = 0; f_3 != 10; f_3++) {volatile int keep = 1; if (keep) break;}

    // N

    for (int i_102 = 0; i_102 < 0; i_102++) {volatile int keep = 1; if (keep) break;}
    for (int i_103 = 0; i_103 < -3; i_103++) {volatile int keep = 1; if (keep) break;}
    for (int i_104 = 0; i_104 < 6; i_104++) {volatile int keep = 1; if (keep) break;}
    for (int i_105 = 0; i_105 < -011; i_105++) {volatile int keep = 1; if (keep) break;}
    for (int i_106 = 0; i_106 < 0xC; i_106++) {volatile int keep = 1; if (keep) break;}

    for (int i_107 = 0; i_107 < VARIABLE_GLOBAL_CONST; i_107++) {volatile int keep = 1; if (keep) break;}
    for (int i_108 = 0; i_108 < stable_variable; i_108++) {volatile int keep = 1; if (keep) break;}
    for (int i_109 = 0; i_109 < stable_variable_const; i_109++) {volatile int keep = 1; if (keep) break;}
    int stable_variable_temp_0 = 0;
    for (int i_110 = 0; i_110 < stable_variable_temp_0; i_110++) {volatile int keep = 1; if (keep) break;}
    int* stable_variable_temp_0_ptr = &stable_variable_temp_0;

    stable_variable = 0;
    _size_t = 0;
    stable_array[0] = 0;
    stable_outer_struct.x = 0;
    for (int i_111 = 0; i_111 < stable_array[0]; i_111++) {volatile int keep = 1; if (keep) break;}
    for (int i_112 = 0; i_112 < stable_array[stable_variable]; i_112++) {volatile int keep = 1; if (keep) break;}
    for (int i_113 = 0; i_113 < stable_array[_size_t]; i_113++) {volatile int keep = 1; if (keep) break;}
    for (int i_114 = 0; i_114 < stable_array[stable_array[0]]; i_114++) {volatile int keep = 1; if (keep) break;}
    for (int i_115 = 0; i_115 < stable_array[stable_array[stable_variable]]; i_115++) {volatile int keep = 1; if (keep) break;}
    for (int i_116 = 0; i_116 < stable_array[stable_array[_size_t]]; i_116++) {volatile int keep = 1; if (keep) break;}
    for (int i_117 = 0; i_117 < stable_array[stable_array[stable_array[0]]]; i_117++) {volatile int keep = 1; if (keep) break;}
    for (int i_118 = 0; i_118 < stable_array[stable_array[stable_outer_struct.x]]; i_118++) {volatile int keep = 1; if (keep) break;}
    for (int i_119 = 0; i_119 < stable_array[stable_outer_struct.x]; i_119++) {volatile int keep = 1; if (keep) break;}
    for (int i_120 = 0; i_120 < stable_array_const[0]; i_120++) {volatile int keep = 1; if (keep) break;}
    for (int i_121 = 0; i_121 < stable_array_static[0]; i_121++) {volatile int keep = 1; if (keep) break;}
    for (int i_122 = 0; i_122 < stable_array_2d[0][0]; i_122++) {volatile int keep = 1; if (keep) break;}
    for (int i_123 = 0; i_123 < stable_array_2d[stable_variable][stable_variable]; i_123++) {volatile int keep = 1; if (keep) break;}
    for (int i_124 = 0; i_124 < stable_array_2d[_size_t][_size_t]; i_124++) {volatile int keep = 1; if (keep) break;}
    for (int i_125 = 0; i_125 < stable_array_2d[stable_array[0]][stable_array[0]]; i_125++) {volatile int keep = 1; if (keep) break;}
    for (int i_126 = 0; i_126 < stable_array_2d[stable_array[stable_variable]][stable_array[stable_variable]]; i_126++) {volatile int keep = 1; if (keep) break;}
    for (int i_127 = 0; i_127 < stable_array_2d[stable_array[_size_t]][stable_array[_size_t]]; i_127++) {volatile int keep = 1; if (keep) break;}
    for (int i_128 = 0; i_128 < stable_array_2d[stable_array[stable_array[0]]][stable_array[stable_array[0]]]; i_128++) {volatile int keep = 1; if (keep) break;}
    for (int i_129 = 0; i_129 < stable_array_2d[stable_array[stable_outer_struct.x]][stable_array[stable_outer_struct.x]]; i_129++) {volatile int keep = 1; if (keep) break;}
    for (int i_130 = 0; i_130 < stable_array_2d[stable_outer_struct.x][stable_outer_struct.x]; i_130++) {volatile int keep = 1; if (keep) break;}
    int stable_array_temp_0[10] = {0};
    int stable_array_temp_1[10] = {0};
    for (int i_131 = 0; i_131 < stable_array_temp_0[0]; i_131++) {volatile int keep = 1; if (keep) break;}
    for (int i_132 = 0; i_132 < stable_array_temp_1[0]; i_132++) {volatile int keep = 1; if (keep) break;}
    int* stable_array_temp_0_ptr = &stable_array_temp_0[1];
    int (*stable_array_temp_1_ptr)[10] = &stable_array_temp_1;

    for (int i_133 = 0; i_133 < stable_outer_struct.x; i_133++) {volatile int keep = 1; if (keep) break;}
    for (int i_134 = 0; i_134 < stable_outer_struct.inner_struct.x; i_134++) {volatile int keep = 1; if (keep) break;}
    for (int i_135 = 0; i_135 < stable_outer_struct_const.x; i_135++) {volatile int keep = 1; if (keep) break;}
    for (int i_136 = 0; i_136 < stable_outer_struct_const.inner_struct.x; i_136++) {volatile int keep = 1; if (keep) break;}
    for (int i_137 = 0; i_137 < stable_outer_struct_static.x; i_137++) {volatile int keep = 1; if (keep) break;}
    for (int i_138 = 0; i_138 < stable_outer_struct_static.inner_struct.x; i_138++) {volatile int keep = 1; if (keep) break;}
    outer_struct_t stable_outer_struct_temp_0 = { 0, { 0 } };
    outer_struct_t stable_outer_struct_temp_1 = { 0, { 0 } };
    for (int i_139 = 0; i_139 < stable_outer_struct_temp_0.x; i_139++) {volatile int keep = 1; if (keep) break;}
    for (int i_140 = 0; i_140 < stable_outer_struct_temp_1.inner_struct.x; i_140++) {volatile int keep = 1; if (keep) break;}
    int* stable_outer_struct_temp_0_ptr = &stable_outer_struct_temp_0.inner_struct.x;
    outer_struct_t* stable_outer_struct_temp_1_ptr = &stable_outer_struct_temp_1;

    for (int32_t i_141 = 0; i_141 < INT16_MAX; i_141++) {volatile int keep = 1; if (keep) break;}
    for (int32_t i_142 = 0; INT16_MAX > i_142; i_142++) {volatile int keep = 1; if (keep) break;}

    receive_stable_t_values_for_n
    (
        stable_variable,
        stable_variable_const,
        stable_outer_struct,
        stable_outer_struct_const
    );

    for (int i_143 = 0; i_143 < _signed_char; i_143++) {volatile int keep = 1; if (keep) break;}
    for (int i_144 = 0; i_144 < _unsigned_char; i_144++) {volatile int keep = 1; if (keep) break;}
    for (int i_145 = 0; i_145 < _short; i_145++) {volatile int keep = 1; if (keep) break;}
    for (int i_146 = 0; i_146 < _unsigned_short; i_146++) {volatile int keep = 1; if (keep) break;}
    for (int i_147 = 0; i_147 < _int8_t; i_147++) {volatile int keep = 1; if (keep) break;}
    for (int i_148 = 0; i_148 < _uint8_t; i_148++) {volatile int keep = 1; if (keep) break;}
    for (int i_149 = 0; i_149 < _int16_t; i_149++) {volatile int keep = 1; if (keep) break;}
    for (int i_150 = 0; i_150 < _uint16_t; i_150++) {volatile int keep = 1; if (keep) break;}

    stable_array[0] = 0;
    for (int i_151 = 0; i_151 > 10; i_151--) {volatile int keep = 1; if (keep) break;}
    for (int i_152 = 0; VARIABLE_GLOBAL_CONST < i_152; i_152--) {volatile int keep = 1; if (keep) break;}
    for (int i_153 = 0; i_153 > stable_variable; i_153--) {volatile int keep = 1; if (keep) break;}
    for (int i_154 = 0; stable_array[0] < i_154; i_154--) {volatile int keep = 1; if (keep) break;}
    for (int i_155 = 0; stable_outer_struct.x < i_155; i_155--) {volatile int keep = 1; if (keep) break;}

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

    variable = 0;
    size_t _size_t_temp = 0;
    size_t* _size_t_temp_ptr = &_size_t_temp;
    stable_array[0] = 0;
    outer_struct.x = 0;
    for (int f_7 = 0; f_7 < stable_array[variable]; f_7++) {volatile int keep = 1; if (keep) break;}
    for (int f_8 = 0; f_8 < stable_array[_size_t_temp]; f_8++) {volatile int keep = 1; if (keep) break;}
    for (int f_9 = 0; f_9 < stable_array[stable_array[variable]]; f_9++) {volatile int keep = 1; if (keep) break;}
    for (int f_10 = 0; f_10 < stable_array[stable_array[_size_t_temp]]; f_10++) {volatile int keep = 1; if (keep) break;}
    for (int f_11 = 0; f_11 < stable_array[stable_array[stable_array[variable]]]; f_11++) {volatile int keep = 1; if (keep) break;}
    for (int f_12 = 0; f_12 < stable_array[stable_array[outer_struct.x]]; f_12++) {volatile int keep = 1; if (keep) break;}
    for (int f_13 = 0; f_13 < stable_array[outer_struct.x]; f_13++) {volatile int keep = 1; if (keep) break;}
    int array_temp_0[10] = {0};
    int array_temp_1[10] = {0};
    int* array_temp_0_ptr = &array_temp_0[1];
    int (*array_temp_1_ptr)[10] = &array_temp_1;
    for (int f_14 = 0; f_14 < array_temp_0[0]; f_14++) {volatile int keep = 1; if (keep) break;}
    for (int f_15 = 0; f_15 < array_temp_1[0]; f_15++) {volatile int keep = 1; if (keep) break;}
    int array_temp_2[10] = {0};
    int array_temp_3[10] = {0};
    for (int f_16 = 0; f_16 < array_temp_2[0]; f_16++)
    {
        int* array_temp_2_ptr = &array_temp_2[1];
        volatile int keep = 1;
        if (keep) break;
    }
    for (int f_17 = 0; f_17 < array_temp_3[0]; f_17++)
    {
        int (*array_temp_3_ptr)[10] = &array_temp_3;
        volatile int keep = 1;
        if (keep) break;
    }
    for (int f_18 = 0; f_18 < stable_array[0]; f_18++) {stable_array[1] = 0; volatile int keep = 1; if (keep) break;}

    for (int f_19 = 0; f_19 < outer_struct.x; f_19++) {volatile int keep = 1; if (keep) break;}
    outer_struct_t outer_struct_temp_0 = { 0, { 0 } };
    int* outer_struct_temp_0_ptr = &outer_struct_temp_0.inner_struct.x;
    for (int f_20 = 0; f_20 < outer_struct_temp_0.x; f_20++) {volatile int keep = 1; if (keep) break;}
    outer_struct_t outer_struct_temp_1 = { 0, { 0 } };
    outer_struct_t outer_struct_temp_2 = { 0, { 0 } };
    for (int f_21 = 0; f_21 < outer_struct_temp_1.x; f_21++)
    {
        int* outer_struct_temp_1_ptr = &outer_struct_temp_1.inner_struct.x;
        volatile int keep = 1;
        if (keep) break;
    }
    for (int f_22 = 0; f_22 < outer_struct_temp_2.inner_struct.x; f_22++)
    {
        outer_struct_t* outer_struct_temp_2_ptr = &outer_struct_temp_2;
        volatile int keep = 1;
        if (keep) break;
    }
    outer_struct_t outer_struct_temp_3 = { 0, { 0 } };
    outer_struct_t outer_struct_temp_4 = { 0, { 0 } };
    outer_struct_t outer_struct_temp_5 = { 0, { 0 } };
    for (int f_23 = 0; f_23 < outer_struct_temp_3.x; f_23++)
    {
        outer_struct_temp_3 = (outer_struct_t){ 0, { 0 } };
        volatile int keep = 1;
        if (keep) break;
    }
    for (int f_24 = 0; f_24 < outer_struct_temp_4.x; f_24++)
    {
        outer_struct_temp_4.inner_struct.x = 0;
        volatile int keep = 1;
        if (keep) break;
    }
    for (int f_25 = 0; f_25 < outer_struct_temp_5.inner_struct.x; f_25++)
    {
        outer_struct_temp_5.x = 0;
        volatile int keep = 1;
        if (keep) break;
    }

    stable_variable = variable;
    for (int f_26 = 0; f_26 > stable_variable; f_26--) {volatile int keep = 1; if (keep) break;}
    stable_variable = 0;
    for (int f_27 = 0; array[stable_variable] < f_27; f_27--) {volatile int keep = 1; if (keep) break;}
    for (uint32_t f_28 = 0; f_28 > UINT32_C(4294967295); f_28--) {volatile int keep = 1; if (keep) break;}
    for (int64_t f_29 = 0; INT64_MAX < f_29; f_29--) {volatile int keep = 1; if (keep) break;}
    const uint64_t uint64_max = UINT64_C(18446744073709551615);
    for (uint64_t f_30 = 0; uint64_max < f_30; f_30--) {volatile int keep = 1; if (keep) break;}

    // decrementing_or_incrementing

    for (int i_156 = 0; i_156 < 10; i_156++) {volatile int keep = 1; if (keep) break;}
    for (int i_157 = 0; i_157 <= 10; i_157++) {volatile int keep = 1; if (keep) break;}
    for (int i_158 = 0; 10 > i_158; i_158++) {volatile int keep = 1; if (keep) break;}
    for (int i_159 = 0; 10 >= i_159; i_159++) {volatile int keep = 1; if (keep) break;}

    for (int f_31 = 0; 10 <= f_31; f_31++) {volatile int keep = 1; if (keep) break;}

    for (int i_160 = 0; i_160 > 10; i_160--) {volatile int keep = 1; if (keep) break;}
    for (int i_161 = 0; i_161 >= 10; i_161--) {volatile int keep = 1; if (keep) break;}
    for (int i_162 = 0; 10 < i_162 ; i_162--) {volatile int keep = 1; if (keep) break;}
    for (int i_163 = 0; 10 <= i_163; i_163--) {volatile int keep = 1; if (keep) break;}

    for (int f_32 = 0; 10 >= f_32; f_32--) {volatile int keep = 1; if (keep) break;}

    for (int i_164 = 0; i_164 < 10; i_164 += 1) {volatile int keep = 1; if (keep) break;}
    for (int i_165 = 0; i_165 <= 10; i_165 += 1) {volatile int keep = 1; if (keep) break;}
    for (int i_166 = 0; 10 > i_166; i_166 += 2) {volatile int keep = 1; if (keep) break;}
    for (int i_167 = 0; 10 >= i_167; i_167 += 2) {volatile int keep = 1; if (keep) break;}

    for (int f_33 = 0; 10 < f_33; f_33 += 1) {volatile int keep = 1; if (keep) break;}

    for (int i_168 = 0; i_168 > 10; i_168 -= 1) {volatile int keep = 1; if (keep) break;}
    for (int i_169 = 0; i_169 >= 10; i_169 -= 1) {volatile int keep = 1; if (keep) break;}
    for (int i_170 = 0; 10 < i_170; i_170 -= 2) {volatile int keep = 1; if (keep) break;}
    for (int i_171 = 0; 10 <= i_171; i_171 -= 2) {volatile int keep = 1; if (keep) break;}

    for (int f_34 = 0; 10 > f_34; f_34 -= 1) {volatile int keep = 1; if (keep) break;}

    for (int i_172 = 0; i_172 > 10; i_172 += -1) {volatile int keep = 1; if (keep) break;}
    for (int i_173 = 0; i_173 >= 10; i_173 += -1) {volatile int keep = 1; if (keep) break;}
    for (int i_174 = 0; 10 < i_174; i_174 += -2) {volatile int keep = 1; if (keep) break;}
    for (int i_175 = 0; 10 <= i_175; i_175 += -2) {volatile int keep = 1; if (keep) break;}

    for (int f_35 = 0; 10 >= f_35; f_35 += -2) {volatile int keep = 1; if (keep) break;}

    for (int i_176 = 0; i_176 < 10; i_176 -= -1) {volatile int keep = 1; if (keep) break;}
    for (int i_177 = 0; i_177 <= 10; i_177 -= -1) {volatile int keep = 1; if (keep) break;}
    for (int i_178 = 0; 10 > i_178; i_178 -= -2) {volatile int keep = 1; if (keep) break;}
    for (int i_179 = 0; 10 >= i_179; i_179 -= -2) {volatile int keep = 1; if (keep) break;}

    for (int f_36 = 0; 10 <= f_36; f_36 -= -2) {volatile int keep = 1; if (keep) break;}

    for (int i_180 = 0; i_180 < 10; i_180 = i_180 + 1) {volatile int keep = 1; if (keep) break;}
    for (int i_181 = 0; i_181 <= 10; i_181 = i_181 + 1) {volatile int keep = 1; if (keep) break;}
    for (int i_182 = 0; 10 > i_182; i_182 = i_182 + 2) {volatile int keep = 1; if (keep) break;}
    for (int i_183 = 0; 10 >= i_183; i_183 = i_183 + 2) {volatile int keep = 1; if (keep) break;}

    for (int f_37 = 0; 10 < f_37; f_37 = f_37 + 1) {volatile int keep = 1; if (keep) break;}

    for (int i_184 = 0; i_184 > 10; i_184 = i_184 - 1) {volatile int keep = 1; if (keep) break;}
    for (int i_185 = 0; i_185 >= 10; i_185 = i_185 - 1) {volatile int keep = 1; if (keep) break;}
    for (int i_186 = 0; 10 < i_186; i_186 = i_186 - 2) {volatile int keep = 1; if (keep) break;}
    for (int i_187 = 0; 10 <= i_187; i_187 = i_187 - 2) {volatile int keep = 1; if (keep) break;}

    for (int f_38 = 0; 10 > f_38; f_38 = f_38 - 1) {volatile int keep = 1; if (keep) break;}

    for (int i_188 = 0; i_188 > 10; i_188 = i_188 + -1) {volatile int keep = 1; if (keep) break;}
    for (int i_189 = 0; i_189 >= 10; i_189 = i_189 + -1) {volatile int keep = 1; if (keep) break;}
    for (int i_190 = 0; 10 < i_190; i_190 = i_190 + -2) {volatile int keep = 1; if (keep) break;}
    for (int i_191 = 0; 10 <= i_191; i_191 = i_191 + -2) {volatile int keep = 1; if (keep) break;}

    for (int f_39 = 0; 10 >= f_39; f_39 = f_39 + -2) {volatile int keep = 1; if (keep) break;}

    for (int i_192 = 0; i_192 < 10; i_192 = i_192 - -1) {volatile int keep = 1; if (keep) break;}
    for (int i_193 = 0; i_193 <= 10; i_193 = i_193 - -1) {volatile int keep = 1; if (keep) break;}
    for (int i_194 = 0; 10 > i_194; i_194 = i_194 - -2) {volatile int keep = 1; if (keep) break;}
    for (int i_195 = 0; 10 >= i_195; i_195 = i_195 - -2) {volatile int keep = 1; if (keep) break;}

    for (int f_40 = 0; 10 <= f_40; f_40 = f_40 - -2) {volatile int keep = 1; if (keep) break;}

    for (int i_196 = 0; i_196 < 10; i_196 = 1 + i_196) {volatile int keep = 1; if (keep) break;}
    for (int i_197 = 0; i_197 <= 10; i_197 = 1 + i_197) {volatile int keep = 1; if (keep) break;}
    for (int i_198 = 0; 10 > i_198; i_198 = 2 + i_198) {volatile int keep = 1; if (keep) break;}
    for (int i_199 = 0; 10 >= i_199; i_199 = 2 + i_199) {volatile int keep = 1; if (keep) break;}

    for (int f_41 = 0; 10 < f_41; f_41 = 1 + f_41) {volatile int keep = 1; if (keep) break;}

    for (int i_200 = 0; i_200 > 10; i_200 = -1 + i_200) {volatile int keep = 1; if (keep) break;}
    for (int i_201 = 0; i_201 >= 10; i_201 = -1 + i_201) {volatile int keep = 1; if (keep) break;}
    for (int i_202 = 0; 10 < i_202; i_202 = -2 + i_202) {volatile int keep = 1; if (keep) break;}
    for (int i_203 = 0; 10 <= i_203; i_203 = -2 + i_203) {volatile int keep = 1; if (keep) break;}

    for (int f_42 = 0; 10 > f_42; f_42 = -1 + f_42) {volatile int keep = 1; if (keep) break;}

    for (unsigned int i_204 = 0; i_204 < 10; i_204++) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_205 = 0; i_205 <= 10; i_205++) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_206 = 0; 10 > i_206; i_206++) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_207 = 0; 10 >= i_207; i_207++) {volatile int keep = 1; if (keep) break;}

    for (unsigned int f_43 = 0; 10 <= f_43; f_43++) {volatile int keep = 1; if (keep) break;}

    for (unsigned int i_208 = 0; i_208 > 10; i_208--) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_209 = 0; i_209 >= 10; i_209--) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_210 = 0; 10 < i_210 ; i_210--) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_211 = 0; 10 <= i_211; i_211--) {volatile int keep = 1; if (keep) break;}

    for (unsigned int f_44 = 0; 10 >= f_44; f_44--) {volatile int keep = 1; if (keep) break;}

    for (unsigned int i_212 = 0; i_212 < 10; i_212 += 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_213 = 0; i_213 <= 10; i_213 += 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_214 = 0; 10 > i_214; i_214 += 2) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_215 = 0; 10 >= i_215; i_215 += 2) {volatile int keep = 1; if (keep) break;}

    for (unsigned int f_45 = 0; 10 < f_45; f_45 += 1) {volatile int keep = 1; if (keep) break;}

    for (unsigned int i_216 = 0; i_216 > 10; i_216 -= 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_217 = 0; i_217 >= 10; i_217 -= 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_218 = 0; 10 < i_218; i_218 -= 2) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_219 = 0; 10 <= i_219; i_219 -= 2) {volatile int keep = 1; if (keep) break;}

    for (unsigned int f_46 = 0; 10 > f_46; f_46 -= 1) {volatile int keep = 1; if (keep) break;}

    for (unsigned int i_220 = 0; i_220 < 10; i_220 = i_220 + 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_221 = 0; i_221 <= 10; i_221 = i_221 + 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_222 = 0; 10 > i_222; i_222 = i_222 + 2) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_223 = 0; 10 >= i_223; i_223 = i_223 + 2) {volatile int keep = 1; if (keep) break;}

    for (unsigned int f_47 = 0; 10 <= f_47; f_47 = f_47 + 2) {volatile int keep = 1; if (keep) break;}

    for (unsigned int i_224 = 0; i_224 > 10; i_224 = i_224 - 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_225 = 0; i_225 >= 10; i_225 = i_225 - 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_226 = 0; 10 < i_226; i_226 = i_226 - 2) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_227 = 0; 10 <= i_227; i_227 = i_227 - 2) {volatile int keep = 1; if (keep) break;}

    for (unsigned int f_48 = 0; 10 >= f_48; f_48 = f_48 - 2) {volatile int keep = 1; if (keep) break;}

    for (unsigned int i_228 = 0; i_228 < 10; i_228 = 1 + i_228) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_229 = 0; i_229 <= 10; i_229 = 1 + i_229) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_230 = 0; 10 > i_230; i_230 = 2 + i_230) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_231 = 0; 10 >= i_231; i_231 = 2 + i_231) {volatile int keep = 1; if (keep) break;}

    for (unsigned int f_49 = 0; 10 < f_49; f_49 = 1 + f_49) {volatile int keep = 1; if (keep) break;}

    _signed_char = 1;
    for (int i_232 = 0; i_232 < 10; i_232 += _signed_char) {volatile int keep = 1; if (keep) break;}
    for (int i_233 = 0; i_233 <= 10; i_233 += _signed_char) {volatile int keep = 1; if (keep) break;}
    for (int i_234 = 0; 10 > i_234; i_234 += _signed_char) {volatile int keep = 1; if (keep) break;}
    for (int i_235 = 0; 10 >= i_235; i_235 += _signed_char) {volatile int keep = 1; if (keep) break;}

    for (int f_50 = 0; 10 < f_50; f_50 += _signed_char) {volatile int keep = 1; if (keep) break;}

    for (int i_236 = 0; i_236 > 10; i_236 -= _signed_char) {volatile int keep = 1; if (keep) break;}
    for (int i_237 = 0; i_237 >= 10; i_237 -= _signed_char) {volatile int keep = 1; if (keep) break;}
    for (int i_238 = 0; 10 < i_238; i_238 -= _signed_char) {volatile int keep = 1; if (keep) break;}
    for (int i_239 = 0; 10 <= i_239; i_239 -= _signed_char) {volatile int keep = 1; if (keep) break;}

    for (int f_51 = 0; 10 >= f_51; f_51 -= _signed_char) {volatile int keep = 1; if (keep) break;}

    _short = -1;
    for (int i_240 = 0; i_240 > 10; i_240 += _short) {volatile int keep = 1; if (keep) break;}
    for (int i_241 = 0; i_241 >= 10; i_241 += _short) {volatile int keep = 1; if (keep) break;}
    for (int i_242 = 0; 10 < i_242; i_242 += _short) {volatile int keep = 1; if (keep) break;}
    for (int i_243 = 0; 10 <= i_243; i_243 += _short) {volatile int keep = 1; if (keep) break;}

    for (int f_52 = 0; 10 >= f_52; f_52 += _short) {volatile int keep = 1; if (keep) break;}

    for (int i_244 = 0; i_244 < 10; i_244 -= _short) {volatile int keep = 1; if (keep) break;}
    for (int i_245 = 0; i_245 <= 10; i_245 -= _short) {volatile int keep = 1; if (keep) break;}
    for (int i_246 = 0; 10 > i_246; i_246 -= _short) {volatile int keep = 1; if (keep) break;}
    for (int i_247 = 0; 10 >= i_247; i_247 -= _short) {volatile int keep = 1; if (keep) break;}

    for (int f_53 = 0; 10 < f_53; f_53 -= _short) {volatile int keep = 1; if (keep) break;}

    stable_variable = 1;
    for (int i_248 = 0; i_248 < 10; i_248 = i_248 + stable_variable) {volatile int keep = 1; if (keep) break;}
    for (int i_249 = 0; i_249 <= 10; i_249 = i_249 + stable_variable) {volatile int keep = 1; if (keep) break;}
    for (int i_250 = 0; 10 > i_250; i_250 = i_250 + stable_variable) {volatile int keep = 1; if (keep) break;}
    for (int i_251 = 0; 10 >= i_251; i_251 = i_251 + stable_variable) {volatile int keep = 1; if (keep) break;}

    for (int f_54 = 0; 10 < f_54; f_54 = f_54 + stable_variable) {volatile int keep = 1; if (keep) break;}
    variable = 1;
    for (int f_55 = 0; 10 >= f_55; f_55 = f_55 + variable) {volatile int keep = 1; if (keep) break;}

    for (int i_252 = 0; i_252 > 10; i_252 = i_252 - stable_variable) {volatile int keep = 1; if (keep) break;}
    for (int i_253 = 0; i_253 >= 10; i_253 = i_253 - stable_variable) {volatile int keep = 1; if (keep) break;}
    for (int i_254 = 0; 10 < i_254; i_254 = i_254 - stable_variable) {volatile int keep = 1; if (keep) break;}
    for (int i_255 = 0; 10 <= i_255; i_255 = i_255 - stable_variable) {volatile int keep = 1; if (keep) break;}

    for (int f_56 = 0; 10 >= f_56; f_56 = f_56 - stable_variable) {volatile int keep = 1; if (keep) break;}

    _int8_t = -2;
    for (int i_256 = 0; i_256 > 10; i_256 = i_256 + _int8_t) {volatile int keep = 1; if (keep) break;}
    for (int i_257 = 0; i_257 >= 10; i_257 = i_257 + _int8_t) {volatile int keep = 1; if (keep) break;}
    for (int i_258 = 0; 10 < i_258; i_258 = i_258 + _int8_t) {volatile int keep = 1; if (keep) break;}
    for (int i_259 = 0; 10 <= i_259; i_259 = i_259 + _int8_t) {volatile int keep = 1; if (keep) break;}

    for (int f_57 = 0; 10 >= f_57; f_57 = f_57 + _int8_t) {volatile int keep = 1; if (keep) break;}

    for (int i_260 = 0; i_260 < 10; i_260 = i_260 - _int8_t) {volatile int keep = 1; if (keep) break;}
    for (int i_261 = 0; i_261 <= 10; i_261 = i_261 - _int8_t) {volatile int keep = 1; if (keep) break;}
    for (int i_262 = 0; 10 > i_262; i_262 = i_262 - _int8_t) {volatile int keep = 1; if (keep) break;}
    for (int i_263 = 0; 10 >= i_263; i_263 = i_263 - _int8_t) {volatile int keep = 1; if (keep) break;}

    for (int f_58 = 0; 10 < f_58; f_58 = f_58 - _int8_t) {volatile int keep = 1; if (keep) break;}

    _int16_t = 2;
    for (int i_264 = 0; i_264 < 10; i_264 = _int16_t + i_264) {volatile int keep = 1; if (keep) break;}
    for (int i_265 = 0; i_265 <= 10; i_265 = _int16_t + i_265) {volatile int keep = 1; if (keep) break;}
    for (int i_266 = 0; 10 > i_266; i_266 = _int16_t + i_266) {volatile int keep = 1; if (keep) break;}
    for (int i_267 = 0; 10 >= i_267; i_267 = _int16_t + i_267) {volatile int keep = 1; if (keep) break;}

    for (int f_59 = 0; 10 < f_59; f_59 = _int16_t + f_59) {volatile int keep = 1; if (keep) break;}

    _int16_t = -2;
    for (int i_268 = 0; i_268 > 10; i_268 = _int16_t + i_268) {volatile int keep = 1; if (keep) break;}
    for (int i_269 = 0; i_269 >= 10; i_269 = _int16_t + i_269) {volatile int keep = 1; if (keep) break;}
    for (int i_270 = 0; 10 < i_270; i_270 = _int16_t + i_270) {volatile int keep = 1; if (keep) break;}
    for (int i_271 = 0; 10 <= i_271; i_271 = _int16_t + i_271) {volatile int keep = 1; if (keep) break;}

    for (int f_60 = 0; 10 >= f_60; f_60 = _int16_t + f_60) {volatile int keep = 1; if (keep) break;}

    // body

    for (int f_61 = 0; f_61 < 10; f_61++) {f_61++; volatile int keep = 1; if (keep) break;}

    // END

    printf("End of test_idiomatize_loops().\n");
}

static int return_int()
{
    return 0;
}

static unsigned char return_unsigned_char()
{
    return 0;
}

static signed long long return_signed_long_long()
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
    for (int i_272 = variable; i_272 < 10; i_272++) {volatile int keep = 1; if (keep) break;}
    for (int i_273 = variable_const; i_273 < 10; i_273++) {volatile int keep = 1; if (keep) break;}
    for (int i_274 = *pointer; i_274 < 10; i_274++) {volatile int keep = 1; if (keep) break;}
    for (int i_275 = *pointer_const_0; i_275 < 10; i_275++) {volatile int keep = 1; if (keep) break;}
    for (int i_276 = *pointer_const_1; i_276 < 10; i_276++) {volatile int keep = 1; if (keep) break;}
    for (int i_277 = **pointer_2d; i_277 < 10; i_277++) {volatile int keep = 1; if (keep) break;}
    for (int i_278 = outer_struct.x; i_278 < 10; i_278++) {volatile int keep = 1; if (keep) break;}
    for (int i_279 = outer_struct.inner_struct.x; i_279 < 10; i_279++) {volatile int keep = 1; if (keep) break;}
    for (int i_280 = outer_struct_const.x; i_280 < 10; i_280++) {volatile int keep = 1; if (keep) break;}
    for (int i_281 = outer_struct_const.inner_struct.x; i_281 < 10; i_281++) {volatile int keep = 1; if (keep) break;}
    for (int i_282 = outer_struct_ptr->x; i_282 < 10; i_282++) {volatile int keep = 1; if (keep) break;}
    for (int i_283 = outer_struct_ptr->inner_struct.x; i_283 < 10; i_283++) {volatile int keep = 1; if (keep) break;}
    for (int i_284 = outer_struct_ptr_const_0->x; i_284 < 10; i_284++) {volatile int keep = 1; if (keep) break;}
    for (int i_285 = outer_struct_ptr_const_0->inner_struct.x; i_285 < 10; i_285++) {volatile int keep = 1; if (keep) break;}
    for (int i_286 = outer_struct_ptr_const_1->x; i_286 < 10; i_286++) {volatile int keep = 1; if (keep) break;}
    for (int i_287 = outer_struct_ptr_const_1->inner_struct.x; i_287 < 10; i_287++) {volatile int keep = 1; if (keep) break;}
}

static void receive_stable_t_values_for_n
(
    int stable_variable,
    const int stable_variable_const,
    outer_struct_t stable_outer_struct,
    const outer_struct_t stable_outer_struct_const
)
{
    for (int i_288 = 0; i_288 < stable_variable; i_288++) {volatile int keep = 1; if (keep) break;}
    for (int i_289 = 0; i_289 < stable_variable_const; i_289++) {volatile int keep = 1; if (keep) break;}
    for (int i_290 = 0; i_290 < stable_outer_struct.x; i_290++) {volatile int keep = 1; if (keep) break;}
    for (int i_291 = 0; i_291 < stable_outer_struct.inner_struct.x; i_291++) {volatile int keep = 1; if (keep) break;}
    for (int i_292 = 0; i_292 < stable_outer_struct_const.x; i_292++) {volatile int keep = 1; if (keep) break;}
    for (int i_293 = 0; i_293 < stable_outer_struct_const.inner_struct.x; i_293++) {volatile int keep = 1; if (keep) break;}

    int* stable_variable_ptr = &stable_variable;
    for (int f_62 = 0; f_62 < stable_variable; f_62++) {volatile int keep = 1; if (keep) break;}
    outer_struct_t* stable_outer_struct_ptr = &stable_outer_struct;
    for (int f_63 = 0; f_63 < stable_outer_struct.x; f_63++) {volatile int keep = 1; if (keep) break;}
}

static void receive_statically_determinable_t_values_for_n
(
    int stable_variable,
    outer_struct_t stable_outer_struct
)
{
    for (int f_64 = 0; f_64 > stable_variable; f_64--) {volatile int keep = 1; if (keep) break;}
    for (int f_65 = 0; stable_outer_struct.x < f_65; f_65--) {volatile int keep = 1; if (keep) break;}
    for (int f_66 = 0; f_66 > stable_outer_struct.inner_struct.x; f_66--) {volatile int keep = 1; if (keep) break;}

    stable_variable = 0;
    stable_outer_struct.x = 0;
    stable_outer_struct.inner_struct.x = 0;
    for (int i_294 = 0; i_294  > stable_variable; i_294 --) {volatile int keep = 1; if (keep) break;}
    for (int i_295 = 0; stable_outer_struct.x < i_295; i_295 --) {volatile int keep = 1; if (keep) break;}
    for (int i_296 = 0; i_296 > stable_outer_struct.inner_struct.x; i_296 --) {volatile int keep = 1; if (keep) break;}
}
