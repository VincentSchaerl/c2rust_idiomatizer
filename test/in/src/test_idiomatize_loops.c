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
    int variable,
    const int variable_const,
    outer_struct_t outer_struct,
    const outer_struct_t outer_struct_const
);

// T-values

extern int variable_global;
#define VARIABLE_GLOBAL_CONST 0
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

    // U-values

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
    int32_t _int32_t = 0;
    uint32_t _uint32_t = 0;
    int64_t _int64_t = 0;
    uint64_t _uint64_t = 0;
    intptr_t _intptr_t = 0;
    uintptr_t _uintptr_t = 0;
    size_t _size_t = 0;

    // BASE CASE

    // basic form

    for (int i_0 = 0; i_0 < 10; i_0++) {volatile int keep = 1; if (keep) break;}

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
    for (int i_74 = _int32_t; i_74 < 10; i_74++) {volatile int keep = 1; if (keep) break;}
    for (int i_75 = _uint32_t; i_75 < 10; i_75++) {volatile int keep = 1; if (keep) break;}
    for (int i_76 = _int64_t; i_76 < 10; i_76++) {volatile int keep = 1; if (keep) break;}
    for (int i_77 = _uint64_t; i_77 < 10; i_77++) {volatile int keep = 1; if (keep) break;}
    for (int i_78 = _intptr_t; i_78 < 10; i_78++) {volatile int keep = 1; if (keep) break;}
    for (int i_79 = _uintptr_t; i_79 < 10; i_79++) {volatile int keep = 1; if (keep) break;}
    for (int i_80 = _size_t; i_80 < 10; i_80++) {volatile int keep = 1; if (keep) break;}

    // relational_operator

    for (int i_81 = 0; i_81 < 10; i_81++) {volatile int keep = 1; if (keep) break;}
    for (int i_82 = 0; i_82 <= 10; i_82++) {volatile int keep = 1; if (keep) break;}
    for (int i_83 = 0; i_83 > 10; i_83--) {volatile int keep = 1; if (keep) break;}
    for (int i_84 = 0; i_84 >= 10; i_84--) {volatile int keep = 1; if (keep) break;}
    for (int i_85 = 0; 10 < i_85; i_85--) {volatile int keep = 1; if (keep) break;}
    for (int i_86 = 0; 10 <= i_86; i_86--) {volatile int keep = 1; if (keep) break;}
    for (int i_87 = 0; 10 > i_87; i_87++) {volatile int keep = 1; if (keep) break;}
    for (int i_88 = 0; 10 >= i_88; i_88++) {volatile int keep = 1; if (keep) break;}

    // N

    for (int i_89 = 0; i_89 < 0; i_89++) {volatile int keep = 1; if (keep) break;}
    for (int i_90 = 0; i_90 < -3; i_90++) {volatile int keep = 1; if (keep) break;}
    for (int i_91 = 0; i_91 < 6; i_91++) {volatile int keep = 1; if (keep) break;}
    for (int i_92 = 0; i_92 < -011; i_92++) {volatile int keep = 1; if (keep) break;}
    for (int i_93 = 0; i_93 < 0xC; i_93++) {volatile int keep = 1; if (keep) break;}

    for (int i_94 = 0; i_94 < VARIABLE_GLOBAL_CONST; i_94++) {volatile int keep = 1; if (keep) break;}
    for (int i_95 = 0; i_95 < variable; i_95++) {volatile int keep = 1; if (keep) break;}
    for (int i_96 = 0; i_96 < variable_const; i_96++) {volatile int keep = 1; if (keep) break;}
    for (int i_97 = 0; i_97 < variable_static; i_97++) {volatile int keep = 1; if (keep) break;}

    for (int i_98 = 0; i_98 < array[0]; i_98++) {volatile int keep = 1; if (keep) break;}
    for (int i_99 = 0; i_99 < array[i]; i_99++) {volatile int keep = 1; if (keep) break;}
    for (int i_100 = 0; i_100 < array_const[0]; i_100++) {volatile int keep = 1; if (keep) break;}
    for (int i_101 = 0; i_101 < array_const[i]; i_101++) {volatile int keep = 1; if (keep) break;}
    for (int i_102 = 0; i_102 < array_static[0]; i_102++) {volatile int keep = 1; if (keep) break;}
    for (int i_103 = 0; i_103 < array_static[i]; i_103++) {volatile int keep = 1; if (keep) break;}

    for (int i_104 = 0; i_104 < outer_struct.x; i_104++) {volatile int keep = 1; if (keep) break;}
    for (int i_105 = 0; i_105 < outer_struct.inner_struct.x; i_105++) {volatile int keep = 1; if (keep) break;}
    for (int i_106 = 0; i_106 < outer_struct_const.x; i_106++) {volatile int keep = 1; if (keep) break;}
    for (int i_107 = 0; i_107 < outer_struct_const.inner_struct.x; i_107++) {volatile int keep = 1; if (keep) break;}
    for (int i_108 = 0; i_108 < outer_struct_static.x; i_108++) {volatile int keep = 1; if (keep) break;}
    for (int i_109 = 0; i_109 < outer_struct_static.inner_struct.x; i_109++) {volatile int keep = 1; if (keep) break;}

    receive_stable_t_values_for_n
    (
        variable,
        variable_const,
        outer_struct,
        outer_struct_const
    );

    for (int i_110 = 0; i_110 < _signed_char; i_110++) {volatile int keep = 1; if (keep) break;}
    for (int i_111 = 0; i_111 < _unsigned_char; i_111++) {volatile int keep = 1; if (keep) break;}
    for (int i_112 = 0; i_112 < _short; i_112++) {volatile int keep = 1; if (keep) break;}
    for (int i_113 = 0; i_113 < _unsigned_short; i_113++) {volatile int keep = 1; if (keep) break;}
    for (int i_114 = 0; i_114 < _int8_t; i_114++) {volatile int keep = 1; if (keep) break;}
    for (int i_115 = 0; i_115 < _uint8_t; i_115++) {volatile int keep = 1; if (keep) break;}
    for (int i_116 = 0; i_116 < _int16_t; i_116++) {volatile int keep = 1; if (keep) break;}
    for (int i_117 = 0; i_117 < _uint16_t; i_117++) {volatile int keep = 1; if (keep) break;}

    // decrementing_or_incrementing

    for (int i_118 = 0; i_118 < 10; i_118++) {volatile int keep = 1; if (keep) break;}
    for (int i_119 = 0; i_119 <= 10; i_119++) {volatile int keep = 1; if (keep) break;}
    for (int i_120 = 0; 10 > i_120; i_120++) {volatile int keep = 1; if (keep) break;}
    for (int i_121 = 0; 10 >= i_121; i_121++) {volatile int keep = 1; if (keep) break;}

    for (int i_122 = 0; i_122 > 10; i_122--) {volatile int keep = 1; if (keep) break;}
    for (int i_123 = 0; i_123 >= 10; i_123--) {volatile int keep = 1; if (keep) break;}
    for (int i_124 = 0; 10 < i_124 ; i_124--) {volatile int keep = 1; if (keep) break;}
    for (int i_125 = 0; 10 <= i_125; i_125--) {volatile int keep = 1; if (keep) break;}

    for (int i_126 = 0; i_126 < 10; i_126 += 1) {volatile int keep = 1; if (keep) break;}
    for (int i_127 = 0; i_127 <= 10; i_127 += 1) {volatile int keep = 1; if (keep) break;}
    for (int i_128 = 0; 10 > i_128; i_128 += 1) {volatile int keep = 1; if (keep) break;}
    for (int i_129 = 0; 10 >= i_129; i_129 += 1) {volatile int keep = 1; if (keep) break;}

    for (int i_130 = 0; i_130 > 10; i_130 -= 1) {volatile int keep = 1; if (keep) break;}
    for (int i_131 = 0; i_131 >= 10; i_131 -= 1) {volatile int keep = 1; if (keep) break;}
    for (int i_132 = 0; 10 < i_132; i_132 -= 1) {volatile int keep = 1; if (keep) break;}
    for (int i_133 = 0; 10 <= i_133; i_133 -= 1) {volatile int keep = 1; if (keep) break;}

    for (int i_134 = 0; i_134 > 10; i_134 += -1) {volatile int keep = 1; if (keep) break;}
    for (int i_135 = 0; i_135 >= 10; i_135 += -1) {volatile int keep = 1; if (keep) break;}
    for (int i_136 = 0; 10 < i_136; i_136 += -1) {volatile int keep = 1; if (keep) break;}
    for (int i_137 = 0; 10 <= i_137; i_137 += -1) {volatile int keep = 1; if (keep) break;}

    for (int i_138 = 0; i_138 < 10; i_138 -= -1) {volatile int keep = 1; if (keep) break;}
    for (int i_139 = 0; i_139 <= 10; i_139 -= -1) {volatile int keep = 1; if (keep) break;}
    for (int i_140 = 0; 10 > i_140; i_140 -= -1) {volatile int keep = 1; if (keep) break;}
    for (int i_141 = 0; 10 >= i_141; i_141 -= -1) {volatile int keep = 1; if (keep) break;}

    for (int i_142 = 0; i_142 < 10; i_142 = i_142 + 1) {volatile int keep = 1; if (keep) break;}
    for (int i_143 = 0; i_143 <= 10; i_143 = i_143 + 1) {volatile int keep = 1; if (keep) break;}
    for (int i_144 = 0; 10 > i_144; i_144 = i_144 + 1) {volatile int keep = 1; if (keep) break;}
    for (int i_145 = 0; 10 >= i_145; i_145 = i_145 + 1) {volatile int keep = 1; if (keep) break;}

    for (int i_146 = 0; i_146 > 10; i_146 = i_146 - 1) {volatile int keep = 1; if (keep) break;}
    for (int i_147 = 0; i_147 >= 10; i_147 = i_147 - 1) {volatile int keep = 1; if (keep) break;}
    for (int i_148 = 0; 10 < i_148; i_148 = i_148 - 1) {volatile int keep = 1; if (keep) break;}
    for (int i_149 = 0; 10 <= i_149; i_149 = i_149 - 1) {volatile int keep = 1; if (keep) break;}

    for (int i_150 = 0; i_150 > 10; i_150 = i_150 + -1) {volatile int keep = 1; if (keep) break;}
    for (int i_151 = 0; i_151 >= 10; i_151 = i_151 + -1) {volatile int keep = 1; if (keep) break;}
    for (int i_152 = 0; 10 < i_152; i_152 = i_152 + -1) {volatile int keep = 1; if (keep) break;}
    for (int i_153 = 0; 10 <= i_153; i_153 = i_153 + -1) {volatile int keep = 1; if (keep) break;}

    for (int i_154 = 0; i_154 < 10; i_154 = i_154 - -1) {volatile int keep = 1; if (keep) break;}
    for (int i_155 = 0; i_155 <= 10; i_155 = i_155 - -1) {volatile int keep = 1; if (keep) break;}
    for (int i_156 = 0; 10 > i_156; i_156 = i_156 - -1) {volatile int keep = 1; if (keep) break;}
    for (int i_157 = 0; 10 >= i_157; i_157 = i_157 - -1) {volatile int keep = 1; if (keep) break;}

    for (int i_158 = 0; i_158 < 10; i_158 = 1 + i_158) {volatile int keep = 1; if (keep) break;}
    for (int i_159 = 0; i_159 <= 10; i_159 = 1 + i_159) {volatile int keep = 1; if (keep) break;}
    for (int i_160 = 0; 10 > i_160; i_160 = 1 + i_160) {volatile int keep = 1; if (keep) break;}
    for (int i_161 = 0; 10 >= i_161; i_161 = 1 + i_161) {volatile int keep = 1; if (keep) break;}

    for (int i_162 = 0; i_162 > 10; i_162 = -1 + i_162) {volatile int keep = 1; if (keep) break;}
    for (int i_163 = 0; i_163 >= 10; i_163 = -1 + i_163) {volatile int keep = 1; if (keep) break;}
    for (int i_164 = 0; 10 < i_164; i_164 = -1 + i_164) {volatile int keep = 1; if (keep) break;}
    for (int i_165 = 0; 10 <= i_165; i_165 = -1 + i_165) {volatile int keep = 1; if (keep) break;}

    for (unsigned int i_166 = 0; i_166 < 10; i_166++) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_167 = 0; i_167 <= 10; i_167++) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_168 = 0; 10 > i_168; i_168++) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_169 = 0; 10 >= i_169; i_169++) {volatile int keep = 1; if (keep) break;}

    for (unsigned int i_170 = 0; i_170 > 10; i_170--) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_171 = 0; i_171 >= 10; i_171--) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_172 = 0; 10 < i_172 ; i_172--) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_173 = 0; 10 <= i_173; i_173--) {volatile int keep = 1; if (keep) break;}

    for (unsigned int i_174 = 0; i_174 < 10; i_174 += 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_175 = 0; i_175 <= 10; i_175 += 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_176 = 0; 10 > i_176; i_176 += 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_177 = 0; 10 >= i_177; i_177 += 1) {volatile int keep = 1; if (keep) break;}

    for (unsigned int i_178 = 0; i_178 > 10; i_178 -= 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_179 = 0; i_179 >= 10; i_179 -= 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_180 = 0; 10 < i_180; i_180 -= 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_181 = 0; 10 <= i_181; i_181 -= 1) {volatile int keep = 1; if (keep) break;}

    for (unsigned int i_182 = 0; i_182 < 10; i_182 = i_182 + 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_183 = 0; i_183 <= 10; i_183 = i_183 + 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_184 = 0; 10 > i_184; i_184 = i_184 + 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_185 = 0; 10 >= i_185; i_185 = i_185 + 1) {volatile int keep = 1; if (keep) break;}

    for (unsigned int i_186 = 0; i_186 > 10; i_186 = i_186 - 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_187 = 0; i_187 >= 10; i_187 = i_187 - 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_188 = 0; 10 < i_188; i_188 = i_188 - 1) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_189 = 0; 10 <= i_189; i_189 = i_189 - 1) {volatile int keep = 1; if (keep) break;}

    for (unsigned int i_190 = 0; i_190 < 10; i_190 = 1 + i_190) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_191 = 0; i_191 <= 10; i_191 = 1 + i_191) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_192 = 0; 10 > i_192; i_192 = 1 + i_192) {volatile int keep = 1; if (keep) break;}
    for (unsigned int i_193 = 0; 10 >= i_193; i_193 = 1 + i_193) {volatile int keep = 1; if (keep) break;}

    _signed_char = 1;
    for (int i_194 = 0; i_194 < 10; i_194 += _signed_char) {volatile int keep = 1; if (keep) break;}
    for (int i_195 = 0; i_195 <= 10; i_195 += _signed_char) {volatile int keep = 1; if (keep) break;}
    for (int i_196 = 0; 10 > i_196; i_196 += _signed_char) {volatile int keep = 1; if (keep) break;}
    for (int i_197 = 0; 10 >= i_197; i_197 += _signed_char) {volatile int keep = 1; if (keep) break;}

    for (int i_198 = 0; i_198 > 10; i_198 -= _signed_char) {volatile int keep = 1; if (keep) break;}
    for (int i_199 = 0; i_199 >= 10; i_199 -= _signed_char) {volatile int keep = 1; if (keep) break;}
    for (int i_200 = 0; 10 < i_200; i_200 -= _signed_char) {volatile int keep = 1; if (keep) break;}
    for (int i_201 = 0; 10 <= i_201; i_201 -= _signed_char) {volatile int keep = 1; if (keep) break;}

    _short = -1;
    for (int i_202 = 0; i_202 > 10; i_202 += _short) {volatile int keep = 1; if (keep) break;}
    for (int i_203 = 0; i_203 >= 10; i_203 += _short) {volatile int keep = 1; if (keep) break;}
    for (int i_204 = 0; 10 < i_204; i_204 += _short) {volatile int keep = 1; if (keep) break;}
    for (int i_205 = 0; 10 <= i_205; i_205 += _short) {volatile int keep = 1; if (keep) break;}

    for (int i_206 = 0; i_206 < 10; i_206 -= _short) {volatile int keep = 1; if (keep) break;}
    for (int i_207 = 0; i_207 <= 10; i_207 -= _short) {volatile int keep = 1; if (keep) break;}
    for (int i_208 = 0; 10 > i_208; i_208 -= _short) {volatile int keep = 1; if (keep) break;}
    for (int i_209 = 0; 10 >= i_209; i_209 -= _short) {volatile int keep = 1; if (keep) break;}

    variable = 1;
    for (int i_210 = 0; i_210 < 10; i_210 = i_210 + variable) {volatile int keep = 1; if (keep) break;}
    for (int i_211 = 0; i_211 <= 10; i_211 = i_211 + variable) {volatile int keep = 1; if (keep) break;}
    for (int i_212 = 0; 10 > i_212; i_212 = i_212 + variable) {volatile int keep = 1; if (keep) break;}
    for (int i_213 = 0; 10 >= i_213; i_213 = i_213 + variable) {volatile int keep = 1; if (keep) break;}

    for (int i_214 = 0; i_214 > 10; i_214 = i_214 - variable) {volatile int keep = 1; if (keep) break;}
    for (int i_215 = 0; i_215 >= 10; i_215 = i_215 - variable) {volatile int keep = 1; if (keep) break;}
    for (int i_216 = 0; 10 < i_216; i_216 = i_216 - variable) {volatile int keep = 1; if (keep) break;}
    for (int i_217 = 0; 10 <= i_217; i_217 = i_217 - variable) {volatile int keep = 1; if (keep) break;}

    _int8_t = -1;
    for (int i_218 = 0; i_218 > 10; i_218 = i_218 + _int8_t) {volatile int keep = 1; if (keep) break;}
    for (int i_219 = 0; i_219 >= 10; i_219 = i_219 + _int8_t) {volatile int keep = 1; if (keep) break;}
    for (int i_220 = 0; 10 < i_220; i_220 = i_220 + _int8_t) {volatile int keep = 1; if (keep) break;}
    for (int i_221 = 0; 10 <= i_221; i_221 = i_221 + _int8_t) {volatile int keep = 1; if (keep) break;}

    for (int i_222 = 0; i_222 < 10; i_222 = i_222 - _int8_t) {volatile int keep = 1; if (keep) break;}
    for (int i_223 = 0; i_223 <= 10; i_223 = i_223 - _int8_t) {volatile int keep = 1; if (keep) break;}
    for (int i_224 = 0; 10 > i_224; i_224 = i_224 - _int8_t) {volatile int keep = 1; if (keep) break;}
    for (int i_225 = 0; 10 >= i_225; i_225 = i_225 - _int8_t) {volatile int keep = 1; if (keep) break;}

    _int16_t = 1;
    for (int i_226 = 0; i_226 < 10; i_226 = _int16_t + i_226) {volatile int keep = 1; if (keep) break;}
    for (int i_227 = 0; i_227 <= 10; i_227 = _int16_t + i_227) {volatile int keep = 1; if (keep) break;}
    for (int i_228 = 0; 10 > i_228; i_228 = _int16_t + i_228) {volatile int keep = 1; if (keep) break;}
    for (int i_229 = 0; 10 >= i_229; i_229 = _int16_t + i_229) {volatile int keep = 1; if (keep) break;}

    _int16_t = -1;
    for (int i_230 = 0; i_230 > 10; i_230 = _int16_t + i_230) {volatile int keep = 1; if (keep) break;}
    for (int i_231 = 0; i_231 >= 10; i_231 = _int16_t + i_231) {volatile int keep = 1; if (keep) break;}
    for (int i_232 = 0; 10 < i_232; i_232 = _int16_t + i_232) {volatile int keep = 1; if (keep) break;}
    for (int i_233 = 0; 10 <= i_233; i_233 = _int16_t + i_233) {volatile int keep = 1; if (keep) break;}

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
    for (int i_234 = variable; i_234 < 10; i_234++) {volatile int keep = 1; if (keep) break;}
    for (int i_235 = variable_const; i_235 < 10; i_235++) {volatile int keep = 1; if (keep) break;}
    for (int i_236 = *pointer; i_236 < 10; i_236++) {volatile int keep = 1; if (keep) break;}
    for (int i_237 = *pointer_const_0; i_237 < 10; i_237++) {volatile int keep = 1; if (keep) break;}
    for (int i_238 = *pointer_const_1; i_238 < 10; i_238++) {volatile int keep = 1; if (keep) break;}
    for (int i_239 = outer_struct.x; i_239 < 10; i_239++) {volatile int keep = 1; if (keep) break;}
    for (int i_240 = outer_struct.inner_struct.x; i_240 < 10; i_240++) {volatile int keep = 1; if (keep) break;}
    for (int i_241 = outer_struct_const.x; i_241 < 10; i_241++) {volatile int keep = 1; if (keep) break;}
    for (int i_242 = outer_struct_const.inner_struct.x; i_242 < 10; i_242++) {volatile int keep = 1; if (keep) break;}
    for (int i_243 = outer_struct_ptr->x; i_243 < 10; i_243++) {volatile int keep = 1; if (keep) break;}
    for (int i_244 = outer_struct_ptr->inner_struct.x; i_244 < 10; i_244++) {volatile int keep = 1; if (keep) break;}
    for (int i_245 = outer_struct_ptr_const_0->x; i_245 < 10; i_245++) {volatile int keep = 1; if (keep) break;}
    for (int i_246 = outer_struct_ptr_const_0->inner_struct.x; i_246 < 10; i_246++) {volatile int keep = 1; if (keep) break;}
    for (int i_247 = outer_struct_ptr_const_1->x; i_247 < 10; i_247++) {volatile int keep = 1; if (keep) break;}
    for (int i_248 = outer_struct_ptr_const_1->inner_struct.x; i_248 < 10; i_248++) {volatile int keep = 1; if (keep) break;}
}

static void receive_stable_t_values_for_n
(
    int variable,
    const int variable_const,
    outer_struct_t outer_struct,
    const outer_struct_t outer_struct_const
)
{
    for (int i_249 = 0; i_249 < variable; i_249++) {volatile int keep = 1; if (keep) break;}
    for (int i_250 = 0; i_250 < variable_const; i_250++) {volatile int keep = 1; if (keep) break;}
    for (int i_251 = 0; i_251 < outer_struct.x; i_251++) {volatile int keep = 1; if (keep) break;}
    for (int i_252 = 0; i_252 < outer_struct.inner_struct.x; i_252++) {volatile int keep = 1; if (keep) break;}
    for (int i_253 = 0; i_253 < outer_struct_const.x; i_253++) {volatile int keep = 1; if (keep) break;}
    for (int i_254 = 0; i_254 < outer_struct_const.inner_struct.x; i_254++) {volatile int keep = 1; if (keep) break;}
}
























/*
static void receive_integers_for_l
(
    int a,
    int* a_p,
    int array[],
    struct OuterStruct outerStruct,
    struct OuterStruct* outerStruct_p,
    OuterStructTypedef outerStructTypedef,
    OuterStructTypedef* outerStructTypedef_p
);

static void receive_integers_for_n
(
    int a,
    int* a_p,
    int array[],
    struct OuterStruct outerStruct,
    struct OuterStruct* outerStruct_p,
    OuterStructTypedef outerStructTypedef,
    OuterStructTypedef* outerStructTypedef_p
);

static int receive_and_return_integers_for_body
(
    int a,
    int* a_p,
    int array[],
    struct OuterStruct outerStruct,
    struct OuterStruct* outerStruct_p,
    OuterStructTypedef outerStructTypedef,
    OuterStructTypedef* outerStructTypedef_p
);

static int a;
static int array0[10];
static struct OuterStruct outerStruct0;
static OuterStructTypedef outerStructTypedef0;

void fill_arr0(int arr[])
{
    for (int i_255 = 0; i_255 < 10; i_255++)
    {
        arr[i_255] = i_255;
    }
}

void fill_arr1(int arr[10])
{
    for (int i_256 = 0; i_256 < 10; i_256++)
    {
        arr[i_256] = i_256;
    }
}

void fill_arr2(int* arr)
{
    for (int i_257 = 0; i_257 < 10; i_257++)
    {
        arr[i_257] = i_257;
    }
}

void fill_arr3(int arr[])
{
    for (int i_258 = 0; i_258 < 10; i_258++)
    {
        *(arr + i_258) = i_258;
    }
}

void fill_arr4(int arr[10])
{
    for (int i_259 = 0; i_259 < 10; i_259++)
    {
        *(arr + i_259) = i_259;
    }
}

void fill_arr5(int* arr)
{
    for (int i_260 = 0; i_260 < 10; i_260++)
    {
        *(arr + i_260) = i_260;
    }
}

int arr4[5];

void _fill_arr0(int arr[], int length)
{
    for (int i_261 = 0; i_261 < length; i_261++)
    {
        arr[i_261] = i_261;
    }
}

void _fill_arr1(const int arr[], long length)
{
    for (int i_262 = 0; i_262 < length; i_262++)
    {
    }
}

void _fill_arr2(int arr[const], int length)
{
    for (int i_263 = 0; i_263 < length; i_263++)
    {
    }
}

void _fill_arr3(const int arr[const], int length)
{
    for (int i_264 = 0; i_264 < length; i_264++)
    {
    }
}

void _fill_arr4(int arr[], size_t length)
{
    for (int i_265 = 0; i_265 < length; i_265++)
    {
        arr[i_265] = i_265;
    }
}

#define LENGTH_0 5
enum { LENGTH_2 = 5 };

int foo0[5];
static int foo1[5];










































static void receive_integers_for_l
(
    int a,
    int* a_p,
    int array[],
    struct OuterStruct outerStruct,
    struct OuterStruct* outerStruct_p,
    OuterStructTypedef outerStructTypedef,
    OuterStructTypedef* outerStructTypedef_p
)
{
    a = 0;
    *a_p = 0;
    for (int i_266 = a; i_266 < 10; i_266++) {}
    for (int i_267 = *a_p; i_267 < 10; i_267++) {}

    a = 0;
    array[0] = 0;
    for (int i_268 = array[0]; i_268 < 10; i_268++) {}
    for (int i_269 = array[a]; i_269 < 10; i_269++) {}

    outerStruct.x = 0;
    outerStruct.innerStruct.x = 0;
    outerStruct_p->x = 0;
    outerStruct_p->innerStruct.x = 0;
    outerStructTypedef.x = 0;
    outerStructTypedef.innerStructTypedef.x = 0;
    outerStructTypedef_p->x = 0;
    outerStructTypedef_p->innerStructTypedef.x = 0;
    for (int i_270 = outerStruct.x; i_270 < 10; i_270++) {}
    for (int i_271 = outerStruct.innerStruct.x; i_271 < 10; i_271++) {}
    for (int i_272 = outerStruct_p->x; i_272 < 10; i_272++) {}
    for (int i_273 = outerStruct_p->innerStruct.x; i_273 < 10; i_273++) {}
    for (int i_274 = outerStructTypedef.x; i_274 < 10; i_274++) {}
    for (int i_275 = outerStructTypedef.innerStructTypedef.x; i_275 < 10; i_275++) {}
    for (int i_276 = outerStructTypedef_p->x; i_276 < 10; i_276++) {}
    for (int i_277 = outerStructTypedef_p->innerStructTypedef.x; i_277 < 10; i_277++) {}
}

static void receive_integers_for_n
(
    int a,
    int* a_p,
    int array[],
    struct OuterStruct outerStruct,
    struct OuterStruct* outerStruct_p,
    OuterStructTypedef outerStructTypedef,
    OuterStructTypedef* outerStructTypedef_p
)
{
    a = 10;
    *a_p = 10;
    for (int i_278 = 0; i_278 < a; i_278++) {}
    for (int i_279 = 0; i_279 < *a_p; i_279++) {}

    a = 0;
    array[0] = 10;
    for (int i_280 = 0; i_280 < array[0]; i_280++) {}
    for (int i_281 = 0; i_281 < array[a]; i_281++) {}

    outerStruct.x = 10;
    outerStruct.innerStruct.x = 10;
    outerStruct_p->x = 10;
    outerStruct_p->innerStruct.x = 10;
    outerStructTypedef.x = 10;
    outerStructTypedef.innerStructTypedef.x = 10;
    outerStruct_p->x = 10;
    outerStruct_p->innerStruct.x = 10;
    for (int i_282 = 0; i_282 < outerStruct.x; i_282++) {}
    for (int i_283 = 0; i_283 < outerStruct.innerStruct.x; i_283++) {}
    for (int i_284 = 0; i_284 < outerStruct_p->x; i_284++) {}
    for (int i_285 = 0; i_285 < outerStruct_p->innerStruct.x; i_285++) {}
    for (int i_286 = 0; i_286 < outerStructTypedef.x; i_286++) {}
    for (int i_287 = 0; i_287 < outerStructTypedef.innerStructTypedef.x; i_287++) {}
    for (int i_288 = 0; i_288 < outerStructTypedef_p->x; i_288++) {}
    for (int i_289 = 0; i_289 < outerStructTypedef_p->innerStructTypedef.x; i_289++) {}
}

static int receive_and_return_integers_for_body
(
    int a,
    int* a_p,
    int array[],
    struct OuterStruct outerStruct,
    struct OuterStruct* outerStruct_p,
    OuterStructTypedef outerStructTypedef,
    OuterStructTypedef* outerStructTypedef_p
)
{
    for (int i_290 = 0; i_290 < 10; i_290++)
    {
        a = i_290;
        *a_p = i_290;

        array[0] = i_290;

        outerStruct.x = i_290;
        outerStruct.innerStruct.x = i_290;
        outerStruct_p->x = i_290;
        outerStruct_p->innerStruct.x = i_290;
        outerStructTypedef.x = i_290;
        outerStructTypedef.innerStructTypedef.x = i_290;
        outerStructTypedef_p->x = i_290;
        outerStructTypedef_p->innerStructTypedef.x = i_290;

        a = array[i_290];
        array[i_290] = a;

        return i_290;
    }
}












































    short _i = 0;
    for (int i_291 = _i; i_291 < 200; i_291++) {int x = 1; if (x) break;}
    int foo2[5];
    for (size_t i_292 = 0; i_292 < sizeof(foo2) / sizeof(foo2[0]); i_292++)
    {
        foo2[i_292] = i_292;
    }
    int _test;
    for (int i_293 = 0; i_293 < 10; i_293++) 
    {
        if (_test) continue;
        printf("Hllo\n");
    }
    for (unsigned int i_294 = 0; i_294 < 10; i_294++) {}
    unsigned long step = 1;
    for (unsigned long i_295 = 0; i_295 < 10; i_295 += step) {}
    for (uint8_t i_296 = 0; i_296 < 10; i_296 += 2) {}
    for (uint64_t i_297 = 0; i_297 < 10; i_297 -= 1) {}
    for (uintptr_t i_298 = 0; i_298 < 10; i_298 --) {}
    for (size_t i_299 = 0; i_299 < 10; i_299 -= 2) {}

    for (int i_300 = 0; i_300 < 5; i_300++)
    {
        foo2[i_300] = i_300;
    }
    const int foo3[5];
    static int foo4[5];

    int foo5[5];
    _fill_arr0(foo5, 5);
    _fill_arr1(foo5, 5);
    _fill_arr2(foo5, 5);
    _fill_arr3(foo5, 5);

    int foo6[LENGTH_0];
    _fill_arr1(foo6, LENGTH_0);
    for (int i_301 = 0; i_301 < LENGTH_0; i_301++);

    int foo7[LENGTH_2];
    _fill_arr1(foo7, LENGTH_2);
    for (int i_302 = 0; i_302 < LENGTH_2; i_302++);

    int foo8[LENGTH_0];
    _fill_arr4(foo8, sizeof(foo8) / sizeof(foo8[0]));
    for (size_t i_303 = 0; i_303 < sizeof(foo8) / sizeof(foo8[0]); i_303++);





    for (int i_304 = 10; i_304 > 0; i_304 += -2){}

    int arr0[10];
    for (int i_305 = 0; i_305 < sizeof(arr0) / sizeof(arr0[0]); i_305++){}
    _fill_arr0(arr0, 10);

    size_t si_ze = sizeof(arr0) / sizeof(arr0[0]);

    size_t _length = 10;
    int arr1[_length];

    int* length_p = &_length;
    int arr2[*length_p];
    _fill_arr0(arr2, 10);
    
    int _help[] = {10};
    int arr3[_help[0]];
    _fill_arr0(arr3, _help[0]);

    struct _help2_t {int len;};
    struct _help2_t _help2 = {10};
    int arr4[_help2.len];
    _fill_arr0(arr4, _help2.len);

    struct _help2_t* _help2_p = &_help2;
    int arr5[_help2_p->len];
    _fill_arr0(arr5, _help2_p->len);

    int b;
    int* b_p = &b;
    const int* b_p0 = &b;
    int* const b_p1 = &b;
    const int* const b_p2 = &b;
    int array1[10];
    struct OuterStruct outerStruct1;
    OuterStructTypedef outerStructTypedef1;

    // for (<T> <I> = <L>; <I> <relational operator> <N>; <I><decrementing or incrementing>)
    // {
    //     <body>
    // }

    // base case

    struct OuterStruct outerStruct2;
    const struct OuterStruct outerStruct3;
    struct OuterStruct* outerStruct2_p0 = &outerStruct2;
    const struct OuterStruct* outerStruct2_p1 = &outerStruct2;
    struct OuterStruct* const outerStruct2_p2 = &outerStruct2;
    const struct OuterStruct* const outerStruct2_p3 = &outerStruct2;
    struct OuterStruct* outerStruct3_p4 = &outerStruct3;
    const struct OuterStruct* outerStruct3_p5 = &outerStruct3;
    struct OuterStruct* const outerStruct3_p6 = &outerStruct3;
    const struct OuterStruct* const outerStruct3_p7 = &outerStruct3;

    int arr[10];

    for (int i_306 = 0; i_306 < 10; i_306++)
    {
        arr[i_306] = i_306;
    }

    fill_arr0(arr);
    fill_arr1(arr);
    fill_arr2(arr);
    fill_arr3(arr);
    fill_arr4(arr);
    fill_arr5(arr);

    int _y = 1;

    for (int i_307 = 0; i_307 < 10; i_307++) {}
    for (int i_308 = 0; i_308 > 10; i_308--) {}
    for (int i_309 = 0; i_309 < 10; i_309 += 1) {}
    for (int i_310 = 0; i_310 > 10; i_310 -= 1) {}
    for (int i_311 = 0; i_311 < 10; i_311 = i_311 + 1) {}
    for (int i_312 = 0; i_312 > 10; i_312 = i_312 - 1) {}
    for (int i_313 = 0; i_313 < 10; i_313 += _y) {}
    for (int i_314 = 0; i_314 > 10; i_314 -= _y) {}
    for (int i_315 = 0; i_315 < 10; i_315 = i_315 + _y) {}
    for (int i_316 = 0; i_316 > 10; i_316 = i_316 - _y) {}

    struct OuterStruct* outerStruct1_p = &outerStruct1;

    for (int i_317 = 0; i_317 < (*outerStruct1_p).x; i_317++)
    {
    }
    
    short x = 5;

    for (int i_318 = 0; i_318 < sizeof(arr) / sizeof(arr[0]); i_318++);

    for (int i_319 = 0; i_319 < 20 / 2; i_319++);

    int __a = 20;
    int __b = 2;

    for (int i_320 = 0; i_320 < __a / __b; i_320++);

    for (int i_321 = x; i_321 < 10; i_321++) {}
    for (int i_322 = 0; i_322 > -10; i_322--) {}

    int steps = 2;

    for (int i_323 = 0; i_323 < 10; i_323 += 2) {}
    for (int i_324 = 0; i_324 > -10; i_324 -= 2) {}

    for (int i_325 = 0; i_325 < 10; i_325 = i_325 + 2) {}
    for (int i_326 = 0; i_326 > .10; i_326 = i_326 - 2) {}

    for (int i_327 = 0; i_327 < 10; i_327 += steps) {}
    for (int i_328 = 0; i_328 > -10; i_328 -= steps) {}

    for (int i_329 = 0; i_329 < 10; i_329 = i_329 + steps) {}
    for (int i_330 = 0; i_330 > -10; i_330 = i_330 - steps) {}

    // L

    for (int i_331 = -1; i_331 < 10; i_331++) {}
    for (int i_332 = 0; i_332 < 10; i_332++) {}
    for (int i_333 = 1; i_333 < 10; i_333++) {}

    a = 0;
    b = 0;
    for (int i_334 = a; i_334 < 10; i_334++) {}
    for (int i_335 = b; i_335 < 10; i_335++) {}

    a = 0;
    array0[0] = 0;
    array1[0] = 0;
    for (int i_336 = array0[0]; i_336 < 10; i_336++) {}
    for (int i_337 = array0[a]; i_337 < 10; i_337++) {}
    for (int i_338 = array1[0]; i_338 < 10; i_338++) {}
    for (int i_339 = array1[a]; i_339 < 10; i_339++) {}

    outerStruct0.x = 0;
    outerStruct0.innerStruct.x = 0;
    outerStructTypedef0.x = 0;
    outerStructTypedef0.innerStructTypedef.x = 0;
    outerStruct1.x = 0;
    outerStruct1.innerStruct.x = 0;
    outerStructTypedef1.x = 0;
    outerStructTypedef1.innerStructTypedef.x = 0;
    for (int i_340 = outerStruct0.x; i_340 < 10; i_340++) {}
    for (int i_341 = outerStruct0.innerStruct.x; i_341 < 10; i_341++) {}
    for (int i_342 = outerStructTypedef0.x; i_342 < 10; i_342++) {}
    for (int i_343 = outerStructTypedef0.innerStructTypedef.x; i_343 < 10; i_343++) {}
    for (int i_344 = outerStruct1.x; i_344 < 10; i_344++) {}
    for (int i_345 = outerStruct1.innerStruct.x; i_345 < 10; i_345++) {}
    for (int i_346 = outerStructTypedef1.x; i_346 < 10; i_346++) {}
    for (int i_347 = outerStructTypedef1.innerStructTypedef.x; i_347 < 10; i_347++) {}

    receive_integers_for_l(a, &a, array0, outerStruct0, &outerStruct0, outerStructTypedef0, &outerStructTypedef0);

    // <relational operator>

    for (int i_348 = 0; i_348 < 10; i_348++) {}
    for (int i_349 = 0; i_349 <= 10; i_349++) {}
    for (int i_350 = 0; i_350 > 10; i_350++) {}
    for (int i_351 = 0; i_351 >= 10; i_351++) {}

    // N

    for (int i_352 = 0; i_352 < -10; i_352++) {}
    for (int i_353 = 0; i_353 < 0; i_353++) {}
    for (int i_354 = 0; i_354 < 10; i_354++) {}

    a = 10;
    b = 10;
    for (int i_355 = 0; i_355 < a; i_355++) {}
    for (int i_356 = 0; i_356 < b; i_356++) {}

    a = 0;
    array0[0] = 10;
    array1[0] = 10;
    for (int i_357 = 0; i_357 < array0[0]; i_357++) {}
    for (int i_358 = 0; i_358 < array0[a]; i_358++) {}
    for (int i_359 = 0; i_359 < array1[0]; i_359++) {}
    for (int i_360 = 0; i_360 < array1[a]; i_360++) {}

    outerStruct0.x = 10;
    outerStruct0.innerStruct.x = 10;
    outerStructTypedef0.x = 10;
    outerStructTypedef0.innerStructTypedef.x = 10;
    outerStruct1.x = 10;
    outerStruct1.innerStruct.x = 10;
    outerStructTypedef1.x = 10;
    outerStructTypedef1.innerStructTypedef.x = 10;
    for (int i_361 = 0; i_361 < outerStruct0.x; i_361++) {}
    for (int i_362 = 0; i_362 < outerStruct0.innerStruct.x; i_362++) {}
    for (int i_363 = 0; i_363 < outerStructTypedef0.x; i_363++) {}
    for (int i_364 = 0; i_364 < outerStructTypedef0.innerStructTypedef.x; i_364++) {}
    for (int i_365 = 0; i_365 < outerStruct1.x; i_365++) {}
    for (int i_366 = 0; i_366 < outerStruct1.innerStruct.x; i_366++) {}
    for (int i_367 = 0; i_367 < outerStructTypedef1.x; i_367++) {}
    for (int i_368 = 0; i_368 < outerStructTypedef1.innerStructTypedef.x; i_368++) {}

    for (int i_369 = 0; i_369 < 10 + 1; i_369++) {}
    for (int i_370 = 0; i_370 < 10 - 1; i_370++) {}
    for (int i_371 = 0; i_371 < 10 * 2; i_371++) {}
    for (int i_372 = 0; i_372 < 10 / 2; i_372++) {}
    for (int i_373 = 0; i_373 < sizeof(array0) / sizeof(array0[0]); i_373++) {}
    for (int i_374 = 0; i_374 < sizeof(array1) / sizeof(array1[0]); i_374++) {}

    receive_integers_for_n(a, &a, array0, outerStruct0, &outerStruct0, outerStructTypedef0, &outerStructTypedef0);

    // decrementing or incrementing

    for (int i_375 = 0; i_375 < 10; i_375++) {}
    for (int i_376 = 0; i_376 > -10; i_376--) {}
    for (int i_377 = 0; i_377 < 10; i_377 += 2) {}
    for (int i_378 = 0; i_378 > -10; i_378 -= 2) {}
    for (int i_379 = 0; i_379 < 10; i_379 = i_379 + 3) {}
    for (int i_380 = 0; i_380 > -10; i_380 = i_380 - 3) {}

    // body

    for (int i_381 = 0; i_381 < 10; i_381++)
    {
        a = i_381;
        b = i_381;
        int c = i_381;
        int d;
        d = i_381;

        array0[0] = i_381;
        array1[0] = i_381;
        int array2[10] = {i_381};
        int array3[10] = {i_381, i_381, i_381, i_381, i_381, i_381, i_381, i_381, i_381, i_381};

        outerStruct0.x = i_381;
        outerStruct0.innerStruct.x = i_381;
        outerStructTypedef0.x = i_381;
        outerStructTypedef0.innerStructTypedef.x = i_381;
        outerStruct1.x = i_381;
        outerStruct1.innerStruct.x = i_381;
        outerStructTypedef1.x = i_381;
        outerStructTypedef1.innerStructTypedef.x = i_381;
        struct OuterStruct outerStruct2 = { i_381, { i_381 } };
        OuterStructTypedef outerStructTypedef2 = { i_381, { i_381 } };
        struct OuterStruct outerStruct3 = { .x = i_381, .innerStruct = { .x = i_381 } };
        OuterStructTypedef outerStructTypedef3 = { .x = i_381, .innerStructTypedef = { .x = i_381 } };

        a = array0[i_381];
        array0[i_381] = a;
        a = array1[i_381];
        array1[i_381] = a;
        struct OuterStruct outerStructs[10];
        outerStructs[i_381].x = a;
        outerStructs[i_381].innerStruct.x = a;

        a = b + i_381;
        a = i_381 - b;
        a = b * i_381;
        b = 1;
        a = i_381 % b;

        if (i_381) {};
        if (!i_381) {};
        if (i_381 > 0) {};

        int* p = array0 + i_381;
        a = *(array0 + i_381);

        switch (i_381)
        {
            case 1:
                break;
        }

        a = (long)i_381;
        a = i_381 > 0 ? i_381 : -i_381;
        char str[100];
        snprintf(str, sizeof(str), "Value of i_381: %d\n", i_381);
        sizeof(i_381);

        receive_and_return_integers_for_body
        (
            i_381,
            &a,
            (int[10]){i_381},
            (struct OuterStruct){ i_381, { i_381 } },
            &outerStruct0,
            (OuterStructTypedef){ i_381, { i_381 } },
            &outerStructTypedef0
        );
        receive_and_return_integers_for_body
        (
            i_381, 
            &a, 
            (int[]){i_381, i_381, i_381, i_381, i_381, i_381, i_381, i_381, i_381, i_381}, 
            (struct OuterStruct){ .x = i_381, .innerStruct = { .x = i_381 } },
            &outerStruct0, 
            (OuterStructTypedef){ .x = i_381, .innerStructTypedef = { .x = i_381 } },
            &outerStructTypedef0
        );
    }

    // index type to usize

    for (int i_382 = 0; i_382 < sizeof(array0) / sizeof(array0[0]); i_382++)
    {
        array0[i_382] = 0;
    }

    // index based to iterator based

    for (int i_383 = 0; i_383 < sizeof(array0) / sizeof(array0[0]); i_383++)
    {
        array0[i_383] = 0;
    }

    for (int i_384 = 0; i_384 < sizeof(array0) / sizeof(array0[0]); i_384++)
    {
        array0[i_384] = i_384;
    }
*/
