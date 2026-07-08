
#ifndef __Light_h__
#define __Light_h__

#include "Common.hpp"
#include "Mesh.hpp"

class Light
{
public:
	virtual ~Light() = default;
	virtual void setPosition(float UNUSED_PARAM(x), float UNUSED_PARAM(y), float UNUSED_PARAM(z)) {};
	virtual void setDirection(float UNUSED_PARAM(x), float UNUSED_PARAM(y), float UNUSED_PARAM(z)) {};
protected:
	Light() = default;
};




#endif
