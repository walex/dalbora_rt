#ifndef __gltf_scene_h__
#define __gltf_scene_h__


#include "test_api.hpp"
#include "Scene.hpp"
#include "Mesh.hpp"

void load_gltf_scene(RhiDevice& device, RhiCommandBuffer& command_buffer,
	const std::string& file_path, const size_t scene_index, Scene& scene);
#endif