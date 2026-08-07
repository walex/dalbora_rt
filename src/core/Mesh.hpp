#ifndef __Mesh_hpp__
#define __Mesh_hpp__

#include "Common.hpp"

class Material;
class Mesh
{

public:
	Mesh(size_t group_id) : m_group_id(group_id) {}
	virtual ~Mesh() = default;
	void set_material_id(size_t id) { m_material_id = id; }
	const size_t get_material_id() const { return m_material_id; }
	void set_vertices(const RhiDevice& device, RhiCommandBuffer& command_buffer,
		const RhiSharedBuffer& data, const size_t data_offset,
		const size_t length, const size_t stride,
		const resource_format format);
	void set_indices(const RhiDevice& device, RhiCommandBuffer& command_buffer,
		const RhiSharedBuffer& src_buffer, const size_t data_offset,
		const size_t length, const size_t stride,
		const resource_format format);
	void set_normals(const RhiDevice& device, RhiCommandBuffer& command_buffer,
		const RhiSharedBuffer& src_buffer, const size_t data_offset,
		const size_t length, const size_t stride,
		const resource_format format);
	void set_uvs(const RhiDevice& device, RhiCommandBuffer& command_buffer,
		const RhiSharedBuffer& src_buffer, const size_t data_offset,
		const size_t length, const size_t stride,
		const resource_format format);

	RhiGPUBuffer& get_vertex_buffer() {
		return *this->vertex_buffer;
	}

	RhiGPUBuffer* get_index_buffer() {
		return this->index_buffere.get();
	}
	size_t get_group_id() const { return m_group_id; }
protected:
	void upload_data(const RhiDevice& device, RhiCommandBuffer& command_buffer,
		std::unique_ptr<RhiGPUBuffer>& src_buffer, const RhiSharedBuffer& src_data,
		const size_t src_buffer_offset, const size_t length,
		const size_t stride, const resource_format format);
private:
	size_t m_material_id;
	std::unique_ptr<RhiGPUBuffer> vertex_buffer;
	std::unique_ptr<RhiGPUBuffer> index_buffere;
	std::unique_ptr<RhiGPUBuffer> normals_buffer;
	std::unique_ptr<RhiGPUBuffer> uvs_buffer;
	size_t m_group_id = 0;
};



//void set_indices();
//void set_normals();
//void set_uv();

class ReadOnlyMesh : public Mesh {};

/*
	void setMaterial(const Material& material) { m_material = &material; }
	const Material* getMaterial() const { return m_material; }
	virtual const RhiBuffer& get_vertices() const = 0;
	virtual const RhiBuffer& get_indices() const = 0;
protected:
	Mesh(const buffer_memory_type type,
		const size_t vertices_length, const size_t vertices_stride,
		const size_t indices_length, const size_t indices_stride)
		: m_type(type)
		, m_vertices_length(vertices_length)
		, m_vertices_stride(vertices_stride)
		, m_indices_length(indices_length)
		, m_indices_stride(indices_stride) {
	}
	size_t get_vertices_length() const { return m_vertices_length; }
	size_t get_indices_length() const { return m_indices_length; }
	size_t get_vertices_stride() const { return m_vertices_stride; }
	size_t get_indices_stride() const { return m_indices_stride; }
private:
	const Material* m_material = nullptr;
	buffer_memory_type m_type;
	size_t m_vertices_length = 0;
	size_t m_vertices_stride = 0;
	size_t m_indices_length = 0;
	size_t m_indices_stride = 0;

};

class SharedMesh : public Mesh {
public:
	SharedMesh(RhiDevice& device, 
		const size_t vertices_length, const size_t vertices_stride,
		const size_t indices_length = 0, const size_t indices_stride = 0)
		: Mesh(buffer_memory_type_shared_rw,
			vertices_length, vertices_stride,
			indices_length, indices_stride) {

		m_vertices.create(device, vertices_length, vertices_stride, resource_format_float3);
		m_indices.create(device, vertices_length, vertices_stride, resource_format_uint16);
	}
	virtual ~SharedMesh() = default;
	void store(uint8_t* vertices, uint8_t* indices = nullptr);
	const RhiBuffer& get_vertices() const override { return m_vertices; }
	const RhiBuffer& get_indices() const override { return m_indices; }
private:
	RhiSharedBuffer m_vertices;
	RhiSharedBuffer m_indices;
};

class ReadOnlyMesh : public Mesh {
public:
	ReadOnlyMesh(RhiDevice& device, 
		const size_t vertices_length, const size_t vertices_stride,
		const size_t indices_length = 0, const size_t indices_stride = 0)
		: Mesh(buffer_memory_type_gpu_only,
			vertices_length, vertices_stride,
			indices_length, indices_stride)
		, m_shared_mesh(std::make_unique<SharedMesh>(device,
			vertices_length, vertices_stride,
			indices_length, indices_stride)) {

		m_vertices.create(device, vertices_length, vertices_stride, resource_format_float3);
		m_indices.create(device, vertices_length, vertices_stride, resource_format_uint16);
	}
	virtual ~ReadOnlyMesh() = default;
	void upload(RhiCommandBuffer& command_buffer,
		uint8_t* vertices, uint8_t* indices = nullptr);
	void release_shared_buffer() { m_shared_mesh.reset(); }
	const RhiBuffer& get_vertices() const override { return m_vertices; }
	const RhiBuffer& get_indices() const override { return m_indices; }
private:
	RhiGPUBuffer m_vertices;
	RhiGPUBuffer m_indices;
	std::unique_ptr<SharedMesh> m_shared_mesh;
};
*/
#endif
