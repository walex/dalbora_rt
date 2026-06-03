#ifndef __rhi_object_hpp__
#define __rhi_object_hpp__

#include "rhi.hpp"
template<typename... Args>
class ICreateRhiObject {
	
	virtual void create(Args... args) = 0;
};

class RhiObject {
	
public:
	virtual ~RhiObject() = default;
protected:
	RhiObject() = default;
};

#endif // __rhi_class_hpp__
