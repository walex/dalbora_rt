#include "gltf_scene.hpp"

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


const uint8_t* get_model_buffer(const tinygltf::Model& model, const tinygltf::Primitive& primitive,
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
	SceneNode& parent_node,	const float4x4& parent_transform,
	SCENE_CALLBACKS& scene_callbacks)
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

	//------------------------------------
	// World transform
	//------------------------------------
	float4x4 world =
		parent_transform * local;

	std::unique_ptr<SceneNode> scene_node = std::make_unique<SceneNode>(&parent_node);
	scene_node->set_world_transform(world);

	//------------------------------------
	// Mesh
	//------------------------------------
	if (node.mesh >= 0)
	{
		const tinygltf::Mesh& mesh =
			model.meshes[node.mesh];

		// Procesar mesh usando 'world'
		std::unique_ptr<MeshGroupNode> mesh_node = std::make_unique<MeshGroupNode>(scene_node.get());
		mesh_node->mesh_group_index = static_cast<size_t>(node.mesh);
		mesh_node->set_world_transform(world);
		scene_node->add_child(std::move(mesh_node));
	}

	//------------------------------------
	// Children
	//------------------------------------
	for (int child : node.children)
	{
		process_node(device, command_buffer,
			model, child,
			*scene_node, world,
			scene_callbacks);
	}

	if (scene_callbacks.on_new_scene_node != nullptr)
		scene_callbacks.on_new_scene_node(device, command_buffer, *scene_node);

	parent_node.add_child(std::move(scene_node));
}

void load_geometries(const RhiDevice& device, RhiCommandBuffer& command_buffer,
	const tinygltf::Model& gltf_model, SCENE_CALLBACKS& scene_callbacks) {

	scene_callbacks.meshes.reserve(gltf_model.meshes.size());
	size_t tmp_buffer_offset = 0;
	for (size_t i = 0; i < gltf_model.meshes.size(); i++) {

		const auto& gltf_mesh = gltf_model.meshes.at(i);		
		std::vector<Mesh*> model_meshes;
		model_meshes.reserve(gltf_mesh.primitives.size());
		for (auto& gltf_primitive : gltf_mesh.primitives) {

			auto& mesh = scene_callbacks.meshes.emplace_back();
			mesh = std::make_shared<Mesh>();

			resource_format format;

			if (scene_callbacks.on_geometry_loaded != nullptr) {
				size_t stride, length;
				for (auto& attr : gltf_primitive.attributes) {
					
					const uint8_t* data = get_model_buffer(gltf_model, gltf_primitive,
						attr.first, length, stride, format);
					scene_callbacks.on_geometry_loaded(device, command_buffer,
						*mesh, attr.first, 
						data, length, 
						stride, format);
				}
				// INDICES
				const uint8_t* data = get_model_buffer(gltf_model, gltf_primitive,
					"__indices__", length, stride, format);
				scene_callbacks.on_geometry_loaded(device, command_buffer,
					*mesh, "__indices__",
					data, length,
					stride, format);
				model_meshes.push_back(mesh.get());
			}
		}	
		if (scene_callbacks.on_model_loaded)
			scene_callbacks.on_model_loaded(device, command_buffer,
				model_meshes);
	}
}

MaterialProps material_from_gltf(const tinygltf::Material& material) {
	
	MaterialProps props;
	return props;
}

void load_materials(const RhiDevice& device, RhiCommandBuffer& command_buffer,
	const tinygltf::Model& gltf_model, SCENE_CALLBACKS& scene_callbacks) {
	
	if (scene_callbacks.on_new_material) {
		scene_callbacks.materials.reserve(gltf_model.materials.size());
		for (size_t i = 0; i < gltf_model.materials.size(); i++) {

			MaterialProps material_props = material_from_gltf(gltf_model.materials.at(i));
			scene_callbacks.on_new_material(device, command_buffer, material_props);
		}
	}
}

void load_gltf_scene(const RhiDevice& device, RhiCommandBuffer& command_buffer,
	const std::string& file_path, const size_t scene_index, SCENE_CALLBACKS& scene_callbacks) {

	tinygltf::Model gltf_model;
	model_3d_from_file(file_path, gltf_model);

	// load geometries
	load_geometries(device, command_buffer, gltf_model, scene_callbacks);
	
	// load materials
	load_materials(device, command_buffer, gltf_model, scene_callbacks);

	size_t local_scene_index =
		gltf_model.scenes.size() > scene_index
		? scene_index
		: 0;

	const tinygltf::Scene& gltf_scene =
		gltf_model.scenes[scene_index];

	float4x4 identity =
		float4x4::Identity();

	for (int rootNode : gltf_scene.nodes)
	{
		process_node(device, command_buffer,
			gltf_model, rootNode,
			scene_callbacks.root_node, identity,
			scene_callbacks);
	}

	compute_scene_bounds(gltf_model, float4x4::Identity(), scene_callbacks.bb_min, scene_callbacks.bb_max);

	if (scene_callbacks.on_scene_loaded)
		scene_callbacks.on_scene_loaded(device, command_buffer, scene_callbacks.root_node);
}