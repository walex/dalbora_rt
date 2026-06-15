#include "Mesh.hpp"
#include "Material.hpp"

void Mesh::upload_data(const RhiDevice& device, RhiCommandBuffer& command_buffer,
	std::unique_ptr<RhiGPUBuffer>& dest_buffer, const RhiSharedBuffer& src_buffer,
	const size_t src_buffer_offset, const size_t length,
	const size_t stride, const resource_format format) {

	dest_buffer = std::make_unique<RhiGPUBuffer>();
	dest_buffer->create(device, length, stride, format);
	dest_buffer->upload(command_buffer, src_buffer, src_buffer_offset, 0, length);
}

void Mesh::set_vertices(const RhiDevice& device, RhiCommandBuffer& command_buffer,
	const RhiSharedBuffer& src_buffer, const size_t data_offset,
	const size_t length, const size_t stride,
	const resource_format format) {

	this->upload_data(device, command_buffer,
		this->vertices_buffer, src_buffer,
		data_offset, length,
		stride, format);
}

void Mesh::set_indices(const RhiDevice& device, RhiCommandBuffer& command_buffer,
	const RhiSharedBuffer& src_buffer, const size_t data_offset,
	const size_t length, const size_t stride,
	const resource_format format) {

	this->upload_data(device, command_buffer,
		this->indices_buffer, src_buffer,
		data_offset, length,
		stride, format);
}

void Mesh::set_normals(const RhiDevice& device, RhiCommandBuffer& command_buffer,
	const RhiSharedBuffer& src_buffer, const size_t data_offset,
	const size_t length, const size_t stride,
	const resource_format format) {

	this->upload_data(device, command_buffer,
		this->normals_buffer, src_buffer,
		data_offset, length,
		stride, format);
}

void Mesh::set_uvs(const RhiDevice& device, RhiCommandBuffer& command_buffer,
	const RhiSharedBuffer& src_buffer, const size_t data_offset,
	const size_t length, const size_t stride,
	const resource_format format) {

	this->upload_data(device, command_buffer,
		this->uvs_buffer, src_buffer,
		data_offset, length,
		stride, format);
}

//void SharedMesh::store(uint8_t* vertices, uint8_t* indices) {
//	
//	RhiSharedBufferMap map = m_vertices.map(0, this->get_vertices_length());
//	memcpy(map.get_data(), vertices, this->get_vertices_length());
//	m_vertices.unmap(map);
//
//	if (indices != nullptr && this->get_indices_length() > 0) {
//		RhiSharedBufferMap map = m_indices.map(0, this->get_indices_length());
//		memcpy(map.get_data(), indices, this->get_indices_length());
//		m_indices.unmap(map);
//	}
//}
//
//void ReadOnlyMesh::upload(RhiCommandBuffer& command_buffer, 
//	uint8_t* vertices, uint8_t* indices) {
//
//	if (vertices == nullptr)
//		throw new std::exception("vertex buffer is null");
//
//	m_shared_mesh->store(vertices, indices);
//	m_vertices.upload(command_buffer, static_cast<const RhiSharedBuffer&>(m_shared_mesh->get_vertices()));
//	if (indices != nullptr && this->get_indices_length() > 0) {
//		m_indices.upload(command_buffer, static_cast<const RhiSharedBuffer&>(m_shared_mesh->get_indices()));
//	}	
//}