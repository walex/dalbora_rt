
#ifndef __RayTracingRenderer_h__
#define __RayTracingRenderer_h__

#include "Common.hpp"

class SurfaceRadiance;
class RayTracingRenderer
{
public:
	RayTracingRenderer();
	virtual ~RayTracingRenderer() = default;
	void draw(const std::vector<SurfaceRadiance*>& radiances);
	void setView(RhiView* view) { render_target_view = view; }
private:
	RhiView* render_target_view = nullptr;
};

#endif // __RayTracingRenderer_h__
