#ifndef __Scene_hpp__
#define __Scene_hpp__

#include "Common.hpp"
#include "Mesh.hpp"

class Camera;
class Light;
class Material;

class Spatial {

public:
	virtual ~Spatial() = default;
	const float4x4& get_world_transform() const { return m_world_transform; }
	void set_world_transform(const float4x4& transform) { m_world_transform = transform; }
protected:
	Spatial() = default;
private:
	float4x4 m_world_transform;
};

enum LeafNodeType {
	LeafNodeType_MeshGroup,
	LeafNodeType_Light
};

class SceneNode : public Spatial {
public:
	IMPLEMENT_COPYABLE_AND_MOVABLE_CLASS(SceneNode)
	SceneNode(SceneNode* parent) : m_parent(parent) {}
	virtual ~SceneNode() = default;
	void add_child(std::unique_ptr<SceneNode> node) { m_childs.push_back(std::move(node)); }
	virtual bool is_leaf() { return false; }
	SceneNode* get_parent() { return m_parent; }
	const std::list<std::unique_ptr<SceneNode>>& get_childs() const { return m_childs; }
private:
	std::list<std::unique_ptr<SceneNode>> m_childs;
	SceneNode* m_parent = nullptr;
	std::string m_name;
};

class LeafNode : public SceneNode {
public:
	virtual ~LeafNode() = default;
	bool is_leaf() override { return true; }
	LeafNodeType get_type() { return m_type; }
protected:
	LeafNode(SceneNode* parent, LeafNodeType type) : SceneNode(parent), m_type(type) {}
private:
	LeafNodeType m_type;
};

class MeshGroupNode : public LeafNode {
public:
	MeshGroupNode(SceneNode* parent) : LeafNode(parent, LeafNodeType_MeshGroup) {}
	virtual ~MeshGroupNode() = default;
	size_t mesh_group_index;
};

class LightNode : public LeafNode {
public:
	LightNode(SceneNode* parent) : LeafNode(parent, LeafNodeType_Light) {}
	virtual ~LightNode() = default;
	size_t mesh_index;
};

class MaterialProps {
	
public:
	IMPLEMENT_COPYABLE_AND_MOVABLE_CLASS(MaterialProps);
	MaterialProps() = default;
};

struct SCENE_CALLBACKS {
	std::vector<std::shared_ptr<Mesh>> meshes;
	std::vector<std::shared_ptr<Material>> materials;
	float3 bb_min, bb_max;
	SceneNode root_node = nullptr;
	std::function<void(const RhiDevice&, RhiCommandBuffer&,
		Mesh&, const std::string&,
		const uint8_t* const, const size_t,
		const size_t, const resource_format)> on_geometry_loaded;
	std::function<void(const RhiDevice&, RhiCommandBuffer&,
		const std::vector<Mesh*>&)> on_model_loaded;
	std::function<void(const RhiDevice&, RhiCommandBuffer&,
		SceneNode&)> on_new_scene_node;
	std::function<void(const RhiDevice&, RhiCommandBuffer&, MaterialProps&)> on_new_material;
	std::function<void(const RhiDevice&, RhiCommandBuffer&, SceneNode&)> on_scene_loaded;
};

class Scene {
public:
	IMPLEMENT_COPYABLE_AND_MOVABLE_CLASS(Scene);

	Scene(): m_root_node(nullptr) {
	
		
	}
	virtual ~Scene() = default;

	void load(const std::string& scene_path, RhiDevice& device,
		RhiCommandQueue& command_queue);	
	void set_max_size(const size_t value) { m_max_size = value; }
	size_t get_max_size() const { return m_max_size; }
	float3 get_bb_min() const { return m_bb_min; }
	float3 get_bb_max() const { return m_bb_max; }
protected:
	size_t m_max_size = 0;
	float3 m_bb_min, m_bb_max;
	SceneNode m_root_node;
	std::vector<std::shared_ptr<Mesh>> m_meshes;
	std::vector<std::shared_ptr<Material>> m_materials;
	SCENE_CALLBACKS m_scene_callbacks;
};

class RayTraceScene : public Scene {
public:
	IMPLEMENT_COPYABLE_AND_MOVABLE_CLASS(RayTraceScene);
	RayTraceScene();
	virtual ~RayTraceScene() = default;
private:

	void new_rt_buffer_for_3d_model(const RhiDevice& device, RhiCommandBuffer& command_buffer,
		const std::vector<Mesh*>& meshes) {

			std::vector<RHI_BUFFER*> vertices_ptr;
			std::vector<RHI_BUFFER*> indices_ptr;
			vertices_ptr.reserve(meshes.size());
			indices_ptr.reserve(meshes.size());
			for (auto* mesh : meshes) {
				vertices_ptr.push_back(mesh->get_vertex_buffer());
				RhiBuffer* index_buffer = mesh->get_index_buffer();
				if (index_buffer != nullptr)
					indices_ptr.push_back(*index_buffer);
				else
					indices_ptr.push_back(nullptr);
			}
			auto& rt_buffer = this->m_rt_buffers.emplace_back();
			this->m_rt_buffers_transforms.emplace_back();
			rt_buffer.create(device, command_buffer, vertices_ptr, indices_ptr);
	}

	void create_rt_instances(const RhiDevice& device, RhiCommandBuffer& command_buffer) {

		this->m_rt_buffers_instances.create(device, command_buffer, this->m_rt_buffers, this->m_rt_buffers_transforms);
		this->m_rt_buffer_instances_views = this->m_rt_buffers_instances.new_view(device);
	}

	void add_rt_instance_transform(const size_t model_id, const float4x4& data) {

		this->m_rt_buffers_transforms.at(model_id).push_back(reinterpret_cast<const float*>(&data));
	}
private:
	std::vector<RhiRayTraceGeometryBuffer> m_rt_buffers;
	std::vector<std::vector<const float*>> m_rt_buffers_transforms;
	RhiRayTraceGeometrydBufferInstances m_rt_buffers_instances;
	RhiView m_rt_buffer_instances_views;
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
