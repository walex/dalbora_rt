#ifndef __gltf_scene_h__
#define __gltf_scene_h__

#include "Common.hpp"
struct SCENE_LOAD_CALLBACKS;
class SceneNode;

void load_gltf_scene(const RhiDevice& device, RhiCommandBuffer& command_buffer,
	const std::string& file_path, const size_t scene_index, 
	SceneNode& root_node, SCENE_LOAD_CALLBACKS& scene_callbacks);
#endif