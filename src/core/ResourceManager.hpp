#ifndef __ResourceManager_hpp__
#define __ResourceManager_hpp__	

#include "Common.hpp"

class ResourceManager {

public:
	ResourceManager(const RhiDevice& device, const std::vector<size_t>& space_slots);
	RhiView new_resource_view(const RhiDevice& device, const RhiBuffer& buffer);
private:
	RhiDescriptorHeap m_descriptor_heap;
	std::vector<size_t> m_space_slots;
};

#endif // !__ResourceManager_hpp__
