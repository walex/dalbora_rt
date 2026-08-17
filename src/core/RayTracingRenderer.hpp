
#ifndef __RayTracingRenderer_h__
#define __RayTracingRenderer_h__

#include "Renderer.hpp"

class RayTracingRenderer: public Renderer
{
public:
	RayTracingRenderer();
	virtual ~RayTracingRenderer() = default;
protected:
	void on_draw(RhiView& surface) override;
	
};

#endif // __RayTracingRenderer_h__
