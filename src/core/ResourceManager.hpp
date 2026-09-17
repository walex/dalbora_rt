#ifndef __ResourceManager_hpp__
#define __ResourceManager_hpp__	

#include "Common.hpp"

class ResourceManager {

public:
	static void set_memory_descriptor(RhiMemoryTable* memory_descriptor);
	static RhiMemoryTable& get_memory_descriptor();
	static RhiView new_resource_view(const RhiDevice& device, const RhiBuffer& buffer);
private:
	static RhiMemoryTable* g_memory_descriptor;
};

#endif // !__ResourceManager_hpp__
