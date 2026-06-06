
#ifndef __SurfaceRadiance_hpp__
#define __SurfaceRadiance_hpp__

#include "Common.hpp"

class SurfaceRadiance
{
};

class SurfaceRadianceGPU : public SurfaceRadiance {
	
public:
	operator RhiRenderTarget& () { return m_render_target; }
private:
	RhiRenderTarget m_render_target;
};

#endif
