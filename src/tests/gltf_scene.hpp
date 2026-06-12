#ifndef __gltf_scene_h__
#define __gltf_scene_h__


#include "test_api.hpp"

class Spatial {

public:
	virtual ~Spatial() = default;
	Eigen::Matrix4f& get_transform() { return m_transform; }
private:
	Eigen::Matrix4f m_transform;
};

class SceneNode : public Spatial {
public:
	virtual ~SceneNode() = default;
	virtual std::list<std::unique_ptr<SceneNode>>& get_childs() { return m_childs; }
	virtual bool is_leaf() { return false; }
private:
	std::list<std::unique_ptr<SceneNode>> m_childs;
	SceneNode* parent = nullptr;
	std::string name;
};

class LeafNode : public SceneNode {
public:
	virtual ~LeafNode() = default;
	virtual std::list<std::unique_ptr<SceneNode>>& get_childs() override { throw std::exception("leaf node has no childs"); }
	bool is_leaf() override { return true; }
};

class Mesh {
public:
	RhiGPUBuffer vertex_buffer;
	std::unique_ptr<RhiGPUBuffer> index_buffer;
	std::unique_ptr<RhiGPUBuffer> normals_buffer;
	std::unique_ptr<RhiGPUBuffer> texture_coords_buffer;
};

class MeshNode : public LeafNode {
public:
	virtual ~MeshNode() = default;
	std::shared_ptr<Mesh> mesh;
};

class LightNode : public MeshNode {
public:
	virtual ~LightNode() = default;
};

class Scene {
public:
	virtual ~Scene() = default;
	Eigen::Vector3f bb_min, bb_max;
	RhiSharedBuffer tmp_buffer;
	SceneNode root_node;
	std::vector<std::shared_ptr<Mesh>> meshes;
	std::vector<RhiRayTraceGeometryBuffer> rt_buffers;
	std::vector<std::vector<float*>> rt_buffers_transforms;
	RhiRayTraceGeometrydBufferInstances rt_buffers_instances;
	RhiView rt_buffer_instances_views;
	size_t max_size = 1024 * 1024 * 6;
};

void load_gltf_scene(RhiDevice& device, RhiCommandBuffer& command_buffer,
	const std::string& file_path, const size_t scene_index, Scene& scene);
#endif