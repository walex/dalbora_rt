#ifndef __gltf_scene_h__
#define __gltf_scene_h__

#include "Scene.hpp"
#include "Mesh.hpp"

void load_gltf_scene(const RhiDevice& device, RhiCommandBuffer& command_buffer,
	const std::string& file_path, const size_t scene_index, SCENE_CALLBACKS& scene_callbacks);
#endif