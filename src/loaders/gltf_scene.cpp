#include "gltf_scene.hpp"
#include "Scene.hpp"
#include "Mesh.hpp"
#include "PBRMaterial.hpp"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.
#include "D:\src\tinygltf\tiny_gltf.h"

void model_3d_from_file(const std::string& filename, tinygltf::Model& model) {

	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;

	bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
	//bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, filename); // for binary glTF(.glb)

	if (!warn.empty()) {
		printf("Warn: %s\n", warn.c_str());
	}

	if (!err.empty()) {
		printf("Err: %s\n", err.c_str());
	}

	if (!ret) {
		printf("Failed to parse glTF: %s\n", filename.c_str());
	}
}

resource_format gltfFormatToDxgiFormat(
	int componentType,
	int type)
{
	if (componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
	{
		switch (type)
		{
		case TINYGLTF_TYPE_SCALAR:
			return resource_format_float;

		case TINYGLTF_TYPE_VEC2:
			return resource_format_float2;

		case TINYGLTF_TYPE_VEC3:
			return resource_format_float3;

		case TINYGLTF_TYPE_VEC4:
			return resource_format_float4;
		}
	}
	else if (componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
		return resource_format_uint16;
	}
	else if (componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
		return resource_format_uint32;
	}

	return resource_format_none;
}

void compute_scene_bounds(
	const tinygltf::Model& model,
	const float4x4& world_matrix,
	float3& scene_min,
	float3& scene_max)
{
	scene_min =
	{
		FLT_MAX,
		FLT_MAX,
		FLT_MAX
	};

	scene_max =
	{
		-FLT_MAX,
		-FLT_MAX,
		-FLT_MAX
	};

	for (const auto& mesh : model.meshes)
	{
		for (const auto& primitive : mesh.primitives)
		{
			auto pos_it = primitive.attributes.find("POSITION");

			if (pos_it == primitive.attributes.end())
				continue;

			const tinygltf::Accessor& accessor =
				model.accessors[pos_it->second];

			if (accessor.minValues.size() < 3 ||
				accessor.maxValues.size() < 3)
			{
				continue;
			}

			float3 local_min(
				static_cast<float>(accessor.minValues[0]),
				static_cast<float>(accessor.minValues[1]),
				static_cast<float>(accessor.minValues[2]));

			float3 local_max(
				static_cast<float>(accessor.maxValues[0]),
				static_cast<float>(accessor.maxValues[1]),
				static_cast<float>(accessor.maxValues[2]));

			float3 corners[8] =
			{
				{local_min.x, local_min.y, local_min.z},
				{local_max.x, local_min.y, local_min.z},
				{local_min.x, local_max.y, local_min.z},
				{local_max.x, local_max.y, local_min.z},
				{local_min.x, local_min.y, local_max.z},
				{local_max.x, local_min.y, local_max.z},
				{local_min.x, local_max.y, local_max.z},
				{local_max.x, local_max.y, local_max.z}
			};

			for (int i = 0; i < 8; ++i)
			{

				float4 p = world_matrix * float4(corners[i].x,
					corners[i].y,
					corners[i].z,
					1.0f);

				scene_min = min(scene_min, p.xyz);
				scene_max = max(scene_max, p.xyz);
			}
		}
	}
}

RhiGPUBuffer gltf_create_buffer(const RhiDevice& device, const RhiCommandBuffer& command_buffer,
	const size_t length, const size_t stride,
	resource_format format, const uint8_t* bytes,
	RhiSharedBuffer& shared_buffer, const size_t shared_buffer_offset) {

	RhiGPUBuffer dest_buffer;
	dest_buffer.create(device, length, stride, format);
	// copy vertices from cpu visible memory to gpu
	if (bytes != nullptr) {
		shared_buffer.copy(bytes, length, shared_buffer_offset);
		// upload to gpu
		dest_buffer.upload(command_buffer, shared_buffer, shared_buffer_offset, 0, length);
	}
	return dest_buffer;
}


const uint8_t* get_mesh_buffer(const tinygltf::Model& model, const tinygltf::Primitive& primitive,
	const std::string& attribute, size_t& length,
	size_t& stride, resource_format& format) {

	int accessor_index;
	if (attribute == "__indices__") {
		if (primitive.indices < 0)
			return nullptr;
		accessor_index = primitive.indices;
	}
	else {
		auto attr = primitive.attributes.find(attribute.c_str());
		if (attr == primitive.attributes.end())
			return nullptr;
		accessor_index = attr->second;
	}

	const auto& accessor =
		model.accessors[accessor_index];

	const auto& view =
		model.bufferViews[accessor.bufferView];

	const auto& buffer =
		model.buffers[view.buffer];

	const uint8_t* data =
		buffer.data.data()
		+ view.byteOffset
		+ accessor.byteOffset;

	stride =
		accessor.ByteStride(view);

	length =
		accessor.count * stride;

	format = gltfFormatToDxgiFormat(
		accessor.componentType,
		accessor.type);

	return data;
}

float4x4 get_node_transforms(tinygltf::Node node) {

	float4x4 M = float4x4::Identity();
	if (!node.translation.empty())
	{
		float3 t(static_cast<float>(node.translation[0]),
			static_cast<float>(node.translation[1]),
			static_cast<float>(node.translation[2])
		);

		float4x4 T;
		T.SetupByTranslation(t);
		M = M * T;
	}

	if (!node.rotation.empty())
	{

		float4 q(static_cast<float>(node.rotation[0]),
			static_cast<float>(node.rotation[1]),
			static_cast<float>(node.rotation[2]),
			static_cast<float>(node.rotation[3])
		);

		float4x4 R;
		R.SetupByQuaternion(q);
		M = M * R;
	}

	if (!node.scale.empty())
	{
		float3 s(static_cast<float>(node.scale[0]),
			static_cast<float>(node.scale[1]),
			static_cast<float>(node.scale[2])
		);

		float4x4 S = float4x4::Identity();
		S.SetupByScale(s);
		M = M * S;
	}
	return M;
}


void process_node(const RhiDevice& device, RhiCommandBuffer& command_buffer,
	const tinygltf::Model& model, const int nodeIndex,
	SceneNode& parent_node,
	SCENE_LOAD_CALLBACKS& scene_callbacks)
{
	const tinygltf::Node& node = model.nodes[nodeIndex];

	float4x4 local = get_node_transforms(node);

	if (!node.matrix.empty())
	{
		float4x4 mt;
		for (int i = 0; i < 16; ++i) {
			((float*)&mt)[i] = static_cast<float>(node.matrix[i]);
		}
		local = local * mt;
	}

	std::unique_ptr<SceneNode> scene_node = std::make_unique<SceneNode>(&parent_node);
	scene_node->set_local_transform(local);

	scene_callbacks.on_new_scene_node(device, command_buffer, *scene_node);

	//------------------------------------
	// Mesh
	//------------------------------------
	if (node.mesh >= 0) {

		// Fix: node.mesh is a group of scene meshes, create as many nodes as meshes in the group
		const std::vector<std::unique_ptr<Mesh>>& meshes = scene_callbacks.get_meshes(node.mesh);
		for (auto& mesh : meshes) {
			auto geo_node = std::make_unique<GeometryNode>(scene_node.get(), *mesh);
			scene_callbacks.on_new_scene_node(device, command_buffer, *geo_node);
			scene_node->add_child(std::move(geo_node));
		}
	}
	

	//------------------------------------
	// Children
	//------------------------------------
	for (int child : node.children)
	{
		process_node(device, command_buffer,
			model, child,
			*scene_node,
			scene_callbacks);
	}

	parent_node.add_child(std::move(scene_node));
}

void load_geometries(const RhiDevice& device, RhiCommandBuffer& command_buffer,
	const tinygltf::Model& gltf_model, SCENE_LOAD_CALLBACKS& scene_callbacks) {

	for (size_t i = 0; i < gltf_model.meshes.size(); i++) {

		const auto& gltf_mesh = gltf_model.meshes.at(i);	
		for (auto& gltf_primitive : gltf_mesh.primitives) {

			std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>(i);
			mesh->set_material_id(static_cast<size_t>(gltf_primitive.material));

			resource_format format;

			size_t stride, length;
			for (auto& attr : gltf_primitive.attributes) {

				const uint8_t* data = get_mesh_buffer(gltf_model, gltf_primitive,
					attr.first, length, stride, format);
				scene_callbacks.on_geometry_attrib_loaded(device, command_buffer,
					*mesh, attr.first,
					data, length,
					stride, format);
			}
			// INDICES
			const uint8_t* data = get_mesh_buffer(gltf_model, gltf_primitive,
				"__indices__", length, stride, format);
			scene_callbacks.on_geometry_attrib_loaded(device, command_buffer,
				*mesh, "__indices__",
				data, length,
				stride, format);

			scene_callbacks.on_geometry_loaded(device, std::move(mesh));
		}
		scene_callbacks.on_geometry_group_loaded(device, command_buffer, i);
	}
}

void load_materials(const RhiDevice& device, RhiCommandBuffer& command_buffer,
	const tinygltf::Model& gltf_model, SCENE_LOAD_CALLBACKS& scene_callbacks) {
	
	for (size_t i = 0; i < gltf_model.materials.size(); i++) {
		PBRMaterialProperties material_properties = {};
		const tinygltf::Material& gltf_material = gltf_model.materials.at(i);

		material_properties.albedo_b = static_cast<float>(gltf_material.pbrMetallicRoughness.baseColorFactor[2]);
		material_properties.albedo_g = static_cast<float>(gltf_material.pbrMetallicRoughness.baseColorFactor[1]);
		material_properties.albedo_r = static_cast<float>(gltf_material.pbrMetallicRoughness.baseColorFactor[0]);
		material_properties.emissive_b = static_cast<float>(gltf_material.emissiveFactor[2]);
		material_properties.emissive_g = static_cast<float>(gltf_material.emissiveFactor[1]);
		material_properties.emissive_r = static_cast<float>(gltf_material.emissiveFactor[0]);
		material_properties.metallic = static_cast<float>(gltf_material.pbrMetallicRoughness.metallicFactor);
		material_properties.roughness = static_cast<float>(gltf_material.pbrMetallicRoughness.roughnessFactor);
		material_properties.ior = 1.5f; // Default IOR for most materials
		material_properties.has_emissive = (material_properties.emissive_r > 0.0f || material_properties.emissive_g > 0.0f || material_properties.emissive_b > 0.0f) ? 1 : 0;
		material_properties.is_transparent = (gltf_material.alphaMode == "BLEND") ? 1 : 0;
		material_properties.albedo_map_id = (gltf_material.pbrMetallicRoughness.baseColorTexture.index >= 0) ? static_cast<uint32_t>(gltf_material.pbrMetallicRoughness.baseColorTexture.index) : 0;
		material_properties.metallic_map_id = (gltf_material.pbrMetallicRoughness.metallicRoughnessTexture.index >= 0) ? static_cast<uint32_t>(gltf_material.pbrMetallicRoughness.metallicRoughnessTexture.index) : 0;
		material_properties.roughness_map_id = (gltf_material.pbrMetallicRoughness.metallicRoughnessTexture.index >= 0) ? static_cast<uint32_t>(gltf_material.pbrMetallicRoughness.metallicRoughnessTexture.index) : 0;
		material_properties.normal_map_id = (gltf_material.normalTexture.index >= 0) ? static_cast<uint32_t>(gltf_material.normalTexture.index) : 0;
		material_properties.ao_map_id = (gltf_material.occlusionTexture.index >= 0) ? static_cast<uint32_t>(gltf_material.occlusionTexture.index) : 0;
		material_properties.emissive_map_id = (gltf_material.emissiveTexture.index >= 0) ? static_cast<uint32_t>(gltf_material.emissiveTexture.index) : 0;
		material_properties.emissive_intensity = 1.0f; // Default emissive intensity, can be adjusted based on your needs
		material_properties.transmission = (gltf_material.alphaMode == "BLEND") ? 1.0f : 0.0f; // Default transmission for transparent materials
		scene_callbacks.on_new_pbr_material(gltf_material.name, material_properties);
	}

	//scene_callbacks.materials.reserve(gltf_model.materials.size());
	//for (size_t i = 0; i < gltf_model.materials.size(); i++) {

	//	MaterialProps material_props = material_from_gltf(gltf_model.materials.at(i));
	//	scene_callbacks.on_new_material(device, command_buffer, material_props);
	//}
}

void load_gltf_scene(const RhiDevice& device, RhiCommandBuffer& command_buffer,
	const std::string& file_path, const size_t scene_index, 
	SceneNode& root_node,SCENE_LOAD_CALLBACKS& scene_callbacks) {

	tinygltf::Model gltf_model;
	model_3d_from_file(file_path, gltf_model);

	// load geometries
	load_geometries(device, command_buffer, gltf_model, scene_callbacks);
	
	// load materials
	load_materials(device, command_buffer, gltf_model, scene_callbacks);

	const tinygltf::Scene& gltf_scene =
		gltf_model.scenes[scene_index];

	float4x4 identity =
		float4x4::Identity();

	for (int rootNode : gltf_scene.nodes)
	{
		process_node(device, command_buffer,
			gltf_model, rootNode,
			root_node, scene_callbacks);
	}

	float3 bb_min, bb_max;
	compute_scene_bounds(gltf_model, float4x4::Identity(), bb_min, bb_max);

	scene_callbacks.on_scene_loaded(device, command_buffer, bb_min, bb_max);
}