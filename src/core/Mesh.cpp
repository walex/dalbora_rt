#include "Mesh.hpp"
#include "Material.hpp"

void SharedMesh::store(uint8_t* vertices, uint8_t* indices) {
	
	RhiSharedBufferMap map = m_vertices.map(0, this->get_vertices_length());
	memcpy(map.get_data(), vertices, this->get_vertices_length());
	m_vertices.unmap(map);

	if (indices != nullptr && this->get_indices_length() > 0) {
		RhiSharedBufferMap map = m_indices.map(0, this->get_indices_length());
		memcpy(map.get_data(), indices, this->get_indices_length());
		m_indices.unmap(map);
	}
}

void ReadOnlyMesh::upload(RhiCommandBuffer& command_buffer, 
	uint8_t* vertices, uint8_t* indices) {

	if (vertices == nullptr)
		throw new std::exception("vertex buffer is null");

	m_shared_mesh->store(vertices, indices);
	m_vertices.upload(command_buffer, static_cast<const RhiSharedBuffer&>(m_shared_mesh->get_vertices()));
	if (indices != nullptr && this->get_indices_length() > 0) {
		m_indices.upload(command_buffer, static_cast<const RhiSharedBuffer&>(m_shared_mesh->get_indices()));
	}	
}