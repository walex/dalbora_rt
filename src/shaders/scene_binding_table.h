#ifndef __SceneBindingTable_hpp__
#define __SceneBindingTable_hpp__

#define MAX_SHADER_ARRAY_ELEMENTS 128
#define STRUCT_ALIGNMENT 256
#define STRUCT_ATTRIBUTES_COUNT 4

#define STRUCT_DATA_SIZE (STRUCT_ATTRIBUTES_COUNT * 4 * MAX_SHADER_ARRAY_ELEMENTS)
#define ALIGN_UP(size, align) (((size) + (align) - 1) & ~((align) - 1))
#define REQUIRED_ELEMENTS ((ALIGN_UP(STRUCT_DATA_SIZE, STRUCT_ALIGNMENT) - STRUCT_DATA_SIZE) / 4)

#if !defined(__HLSL_VERSION) && !defined(__cplusplus_shader__)
#include "Common.hpp"
#define SHADER_STRUCT_ALIGN alignas(STRUCT_ALIGNMENT)
#else
#define SHADER_STRUCT_ALIGN 
#endif

struct SHADER_STRUCT_ALIGN _SceneBindingTable {
    uint camera_heap_slot_id[MAX_SHADER_ARRAY_ELEMENTS];
    uint geometry_instance_slot_id[MAX_SHADER_ARRAY_ELEMENTS];
    uint geometry_primitive_slot_id[MAX_SHADER_ARRAY_ELEMENTS];
    uint material_slot_id[MAX_SHADER_ARRAY_ELEMENTS];

#if REQUIRED_ELEMENTS > 0
    uint padding[REQUIRED_ELEMENTS];
#endif
};

#endif