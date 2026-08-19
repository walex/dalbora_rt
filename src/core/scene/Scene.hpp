#ifndef __Scene_hpp__
#define __Scene_hpp__

#include "LightNode.hpp"
#include "GeometryNode.hpp"
#include "Mesh.hpp"
#include "Material.hpp"

struct PBRMaterialProperties;
class BaseCamera;
struct SCENE_LOAD_CALLBACKS {
	
	virtual void on_geometry_attrib_loaded(const RhiDevice& UNUSED_PARAM(device), RhiCommandBuffer& UNUSED_PARAM(command_buffer),
		Mesh& UNUSED_PARAM(mesh), const std::string& UNUSED_PARAM(name),
		const uint8_t* const UNUSED_PARAM(data), const size_t UNUSED_PARAM(length),
		const size_t UNUSED_PARAM(stride), const resource_format UNUSED_PARAM(format)) {}

	virtual void on_geometry_loaded(const RhiDevice& UNUSED_PARAM(device), std::unique_ptr<Mesh> UNUSED_PARAM(mesh)) {}

	virtual void on_geometry_group_loaded(const RhiDevice& UNUSED_PARAM(device), RhiCommandBuffer& UNUSED_PARAM(command_buffer), 
		size_t UNUSED_PARAM(group_id)) {}

	virtual void on_new_scene_node(const RhiDevice& UNUSED_PARAM(device), RhiCommandBuffer& UNUSED_PARAM(command_buffer),
		SceneNode& UNUSED_PARAM(node)) {}

	virtual void on_new_pbr_material(const std::string& UNUSED_PARAM(name), const PBRMaterialProperties& UNUSED_PARAM(material_properties)) {}

	virtual void on_scene_loaded(const RhiDevice& UNUSED_PARAM(device), RhiCommandBuffer& UNUSED_PARAM(command_buffer), const float3 UNUSED_PARAM(bb_min), const float3 UNUSED_PARAM(bb_max)) {}

	virtual const std::vector<std::unique_ptr<Mesh>>& get_meshes(const size_t UNUSED_PARAM(group_id)) const = 0;
};

class Renderer;
class BaseCamera;
class Scene: public SCENE_LOAD_CALLBACKS {
public:
	IMPLEMENT_COPYABLE_AND_MOVABLE_CLASS(Scene);

	Scene(): m_root_node(nullptr) {
	
		
	}
	virtual ~Scene() = default;

	virtual void load(const std::string& scene_path, RhiDevice& device,
		RhiCommandQueue& command_queue);
	void set_max_size(const size_t value) { m_max_size = value; }
	size_t get_max_size() const { return m_max_size; }
	float3 get_bb_min() const { return m_bb_min; }
	float3 get_bb_max() const { return m_bb_max; }

	virtual void on_geometry_attrib_loaded(const RhiDevice& device, RhiCommandBuffer& command_buffer,
		Mesh& mesh, const std::string& name,
		const uint8_t* const data, const size_t length,
		const size_t stride, const resource_format format) override;

	virtual void on_geometry_loaded(const RhiDevice& device, std::unique_ptr<Mesh> mesh) override;

	virtual void on_new_scene_node(const RhiDevice& device, RhiCommandBuffer& command_buffer,
		SceneNode& node) override;

	virtual void on_scene_loaded(const RhiDevice& device, RhiCommandBuffer& command_buffer,
		const float3 bb_min, const float3 bb_max) override;

	const std::vector<std::unique_ptr<Mesh>>& get_meshes(const size_t group_id) const override { return m_meshes.at(group_id); }
	
	SceneNode& get_root_node() { return m_root_node; }
	
	void set_camera(BaseCamera* camera) { m_camera = camera; }

	virtual void draw_scene(Renderer& render, RhiView& surface_view, const RHI_VIEWPORT& viewport);
protected:
	size_t m_max_size = 0;
	float3 m_bb_min, m_bb_max;
	SceneNode m_root_node;
	RhiSharedBuffer m_tmp_buffer;
	size_t tmp_buffer_offset = 0;
	std::map<size_t, std::vector<std::unique_ptr<Mesh>>> m_meshes;
	std::vector<std::unique_ptr<Material>> m_materials;
	BaseCamera* m_camera = nullptr;
};


#endif
