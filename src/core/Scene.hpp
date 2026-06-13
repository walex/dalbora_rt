#ifndef __Scene_hpp__
#define __Scene_hpp__

#include "Common.hpp"

class Mesh;
class Camera;
class Light;
class Spatial {

public:
	virtual ~Spatial() = default;
	Eigen::Matrix4f& get_world_transform() { return m_world_transform; }
private:
	Eigen::Matrix4f m_world_transform;
};

class SceneNode : public Spatial {
public:
	SceneNode(SceneNode* parent) : m_parent(parent) {}
	virtual ~SceneNode() = default;
	void add_child(std::unique_ptr<SceneNode> node) { m_childs.push_back(std::move(node)); }
	virtual bool is_leaf() { return false; }
	SceneNode* get_parent() { return m_parent; }
private:
	std::list<std::unique_ptr<SceneNode>> m_childs;
	SceneNode* m_parent = nullptr;
	std::string m_name;
};

class LeafNode : public SceneNode {
public:
	LeafNode(SceneNode* parent) : SceneNode(parent) {}
	virtual ~LeafNode() = default;
	bool is_leaf() override { return true; }
};

class MeshNode : public LeafNode {
public:
	MeshNode(SceneNode* parent) : LeafNode(parent) {}
	virtual ~MeshNode() = default;
	std::shared_ptr<Mesh> mesh;
};

class LightNode : public MeshNode {
public:
	LightNode(SceneNode* parent) : MeshNode(parent) {}
	virtual ~LightNode() = default;
};

class Scene {
public:
	Scene(): root_node(nullptr) {}
	virtual ~Scene() = default;

	void init_rt_buffers(size_t buffer_length) {
		
		if (this->m_is_rt_scene == true) {
			this->rt_buffers.resize(buffer_length);
			this->rt_buffers_transforms.resize(buffer_length);
		}
	}

	void fill_rt_buffer(RhiDevice& device, RhiCommandBuffer& command_buffer, 
		size_t buffer_index, std::vector<RHI_BUFFER*>& vertices_ptr, 
		std::vector<RHI_BUFFER*>& indices_ptr) {

		if (this->m_is_rt_scene == true) {
			auto& rt_buffer = this->rt_buffers.at(buffer_index);
			rt_buffer.create(device, command_buffer, vertices_ptr, indices_ptr);
		}
	}

	void create_rt_instances(RhiDevice& device, RhiCommandBuffer& command_buffer) {

		if (this->m_is_rt_scene == true) {
			this->rt_buffers_instances.create(device, command_buffer, this->rt_buffers, this->rt_buffers_transforms);
			this->rt_buffer_instances_views = this->rt_buffers_instances.new_view(device);
		}
	}

	void add_rt_instance_trasnform(const size_t model_id, const float* data) {

		if (this->m_is_rt_scene == true) {
			this->rt_buffers_transforms.at(model_id).push_back(data);
		}
	}

	void enable_rt_features(bool value) { m_is_rt_scene = value; }

	Eigen::Vector3f bb_min, bb_max;
	RhiSharedBuffer tmp_buffer;
	SceneNode root_node;
	std::vector<std::shared_ptr<Mesh>> meshes;
	std::vector<RhiRayTraceGeometryBuffer> rt_buffers;
	std::vector<std::vector<const float*>> rt_buffers_transforms;
	RhiRayTraceGeometrydBufferInstances rt_buffers_instances;
	RhiView rt_buffer_instances_views;
	size_t max_size = 1024 * 1024 * 6;
private:
	bool m_is_rt_scene = false;
};
class StaticScene
{
public:
	void addGeometry(const Mesh* const geometry) { m_geometries.push_back(geometry); }
	void addCamera(const Camera* const camera) { m_cameras.push_back(camera); }
	void addLight(const Light* const light) { m_ligths.push_back(light); }
private: 
	std::vector<const Mesh*> m_geometries;
	std::vector<const Camera*> m_cameras;
	std::vector<const Light*> m_ligths;

};

#endif
