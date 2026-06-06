
#ifndef __AreaLight_Circular_h__
#define __AreaLight_Circular_h__

#include "Common.hpp"
#include "AreaLight.hpp"

class AreaLight_Circular : public AreaLight
{
public:
	AreaLight_Circular(RhiDevice& device) : AreaLight(device) {}
	virtual ~AreaLight_Circular() = default;
};



#endif
