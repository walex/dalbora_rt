
#ifndef __Render_h__
#define __Render_h__

#include "Common.hpp"

class SurfaceRadiance;
class Render
{
public:
	void draw(std::shared_ptr<SurfaceRadiance> radiance);
};

#endif
