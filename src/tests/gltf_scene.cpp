#include "gltf_scene.hpp"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.
#include "../../../tinygltf/tiny_gltf.h"

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
	const Eigen::Matrix4f& world_matrix,
	Eigen::Vector3f& scene_min,
	Eigen::Vector3f& scene_max)
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

			Eigen::Vector3f local_min(
				static_cast<float>(accessor.minValues[0]),
				static_cast<float>(accessor.minValues[1]),
				static_cast<float>(accessor.minValues[2]));

			Eigen::Vector3f local_max(
				static_cast<float>(accessor.maxValues[0]),
				static_cast<float>(accessor.maxValues[1]),
				static_cast<float>(accessor.maxValues[2]));

			Eigen::Vector3f corners[8] =
			{
				{local_min.x(), local_min.y(), local_min.z()},
				{local_max.x(), local_min.y(), local_min.z()},
				{local_min.x(), local_max.y(), local_min.z()},
				{local_max.x(), local_max.y(), local_min.z()},
				{local_min.x(), local_min.y(), local_max.z()},
				{local_max.x(), local_min.y(), local_max.z()},
				{local_min.x(), local_max.y(), local_max.z()},
				{local_max.x(), local_max.y(), local_max.z()}
			};

			for (int i = 0; i < 8; ++i)
			{
				Eigen::Vector4f p(
					corners[i].x(),
					corners[i].y(),
					corners[i].z(),
					1.0f);

				p = world_matrix * p;

				scene_min =
					scene_min.cwiseMin(p.head<3>());

				scene_max =
					scene_max.cwiseMax(p.head<3>());
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

Eigen::Matrix4f get_node_transforms(tinygltf::Node node) {

	Eigen::Matrix4f M = Eigen::Matrix4f::Identity();
	if (!node.translation.empty())
	{
		Eigen::Matrix4f T = Eigen::Matrix4f::Identity();

		T(0, 3) = static_cast<float>(node.translation[0]);
		T(1, 3) = static_cast<float>(node.translation[1]);
		T(2, 3) = static_cast<float>(node.translation[2]);

		M *= T;
	}

	if (!node.rotation.empty())
	{
		Eigen::Quaternionf Q(
			node.rotation[3], // w
			node.rotation[0], // x
			node.rotation[1], // y
			node.rotation[2]); // z

		Eigen::Matrix4f R = Eigen::Matrix4f::Identity();

		R.block<3, 3>(0, 0) = Q.toRotationMatrix();


		M *= R;
	}

	if (!node.scale.empty())
	{
		Eigen::Matrix4f S = Eigen::Matrix4f::Identity();
		S.block<3, 3>(0, 0) =
			Eigen::Scaling(
				(float)node.scale[0],
				(float)node.scale[1],
				(float)node.scale[2]);

		M *= S;
	}
	return M;
}

void process_node(
	const tinygltf::Model& model,
	const int nodeIndex,
	SceneNode& parent_node,
	const Eigen::Matrix4f& parent_transform,
	Scene& scene)
{
	const tinygltf::Node& node = model.nodes[nodeIndex];

	Eigen::Matrix4f local = Eigen::Matrix4f::Identity();

	if (!node.matrix.empty())
	{
		local = Eigen::Map<const Eigen::Matrix<double, 4, 4, Eigen::RowMajor>>
			(
				node.matrix.data()
			).cast<float>();
	}
	else
	{
		local = get_node_transforms(node);
	}

	//------------------------------------
	// World transform
	//------------------------------------
	Eigen::Matrix4f world =
		parent_transform * local;

	std::unique_ptr<SceneNode> scene_node = std::make_unique<SceneNode>(&parent_node);
	scene_node->get_world_transform() = world;

	//------------------------------------
	// Mesh
	//------------------------------------
	if (node.mesh >= 0)
	{
		const tinygltf::Mesh& mesh =
			model.meshes[node.mesh];

		// Procesar mesh usando 'world'
		std::unique_ptr<MeshNode> mesh_node = std::make_unique<MeshNode>(scene_node.get());
		mesh_node->mesh = scene.meshes.at(node.mesh);
		auto& t = mesh_node->get_world_transform();
		t = world;
		scene.add_rt_instance_trasnform(static_cast<size_t>(node.mesh), t.data());
		scene_node->add_child(std::move(mesh_node));
	}

	//------------------------------------
	// Children
	//------------------------------------
	for (int child : node.children)
	{
		process_node(
			model,
			child,
			*scene_node,
			world,
			scene);
	}

	parent_node.add_child(std::move(scene_node));
}

void create_geometry_buffers(RhiDevice& device, RhiCommandBuffer& command_buffer,
	const tinygltf::Model& gltf_model, Scene& scene) {

	size_t tmp_buffer_offset = 0;
	scene.tmp_buffer.create(device, scene.max_size);

	std::vector<RHI_BUFFER*> vertices_ptr;
	std::vector<RHI_BUFFER*> indices_ptr;
	//std::vector<RHI_BUFFER*> normals_ptr;
	//std::vector<RHI_BUFFER*> tex_coords_ptr;

	for (size_t i = 0; i < gltf_model.meshes.size(); i++) {
		const auto& gltf_mesh = gltf_model.meshes.at(i);
		for (auto& gltf_primitive : gltf_mesh.primitives) {

			auto& mesh = scene.meshes.emplace_back();
			mesh = std::make_shared<Mesh>();

			resource_format format;

			// POSITION
			size_t vertex_stride, vertices_length;
			const uint8_t* vertices = get_model_buffer(gltf_model, gltf_primitive,
				"POSITION", vertices_length,
				vertex_stride, format);

			if (!vertices)
				throw std::exception("vertex buffer not found");

			// copy vertices from cpu visible memory to gpu					
			scene.tmp_buffer.copy(vertices, vertices_length, tmp_buffer_offset);
			mesh->vertex_buffer.create(device, vertices_length, vertex_stride, format);
			mesh->vertex_buffer.upload(command_buffer, scene.tmp_buffer, tmp_buffer_offset, 0, vertices_length);
			tmp_buffer_offset += vertices_length;
			vertices_ptr.emplace_back(mesh->vertex_buffer);

			// NORMALS
			size_t normal_stride, normals_length;
			const uint8_t* normals = get_model_buffer(gltf_model, gltf_primitive,
				"NORMAL", normals_length,
				normal_stride, format);
			if (normals != nullptr) {
				// copy normals from cpu visible memory to gpu

				tmp_buffer_offset += normals_length;
			}

			// TEXTURE COORDS
			size_t tex_stride, tex_length;
			const uint8_t* tex_coords = get_model_buffer(gltf_model, gltf_primitive,
				"TEXCOORD_0", tex_length,
				tex_stride, format);
			if (tex_coords != nullptr) {
				// copy normals from cpu visible memory to gpu

				tmp_buffer_offset += tex_length;
			}

			// INDICES
			size_t indices_stride, indices_length;
			const uint8_t* indices = get_model_buffer(gltf_model, gltf_primitive,
				"__indices__", indices_length,
				indices_stride, format);
			if (indices != nullptr) {

				// copy tex coords from cpu visible memory to gpu						
				scene.tmp_buffer.copy(indices, indices_length, tmp_buffer_offset);
				mesh->index_buffer = std::make_unique<RhiGPUBuffer>();
				mesh->index_buffer->create(device, indices_length, indices_stride, format);
				mesh->index_buffer->upload(command_buffer, scene.tmp_buffer, tmp_buffer_offset, 0, indices_length);
				tmp_buffer_offset += indices_length;
				indices_ptr.push_back(*mesh->index_buffer);
			}
			else
				indices_ptr.push_back(nullptr);
		}
		scene.fill_rt_buffer(device, command_buffer, i, vertices_ptr, indices_ptr);
	}
}

void load_gltf_scene(RhiDevice& device, RhiCommandBuffer& command_buffer,
	const std::string& file_path, const size_t scene_index, Scene& scene) {	

	tinygltf::Model gltf_model;
	model_3d_from_file(file_path, gltf_model);

	scene.meshes.resize(gltf_model.meshes.size());
	scene.init_rt_buffers(gltf_model.meshes.size());
	create_geometry_buffers(device, command_buffer, gltf_model, scene);	

	size_t local_scene_index =
		gltf_model.scenes.size() > scene_index
		? scene_index
		: 0;

	const tinygltf::Scene& gltf_scene =
		gltf_model.scenes[scene_index];

	Eigen::Matrix4f identity =
		Eigen::Matrix4f::Identity();

	for (int rootNode : gltf_scene.nodes)
	{
		process_node(
			gltf_model,
			rootNode,
			scene.root_node,
			identity,
			scene);
	}

	scene.create_rt_instances(device, command_buffer);
	compute_scene_bounds(gltf_model, Eigen::Matrix4f::Identity(), scene.bb_min, scene.bb_max);
}