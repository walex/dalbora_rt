
#ifndef __Camera_hpp__
#define __Camera_hpp__

#include "Common.hpp"

class Sampler;
class Samples;
class Rays;
class Camera
{
public:
	virtual ~Camera() = default;
	void setSampler(const Sampler* const sampler) { m_sampler = sampler; }
	Samples generateSamples(const size_t grid[4]) const;
	virtual void setPosition(float UNUSED_PARAM(x), float UNUSED_PARAM(x), float UNUSED_PARAM(z)) {};
	virtual void setLookAt(float UNUSED_PARAM(x), float UNUSED_PARAM(x), float UNUSED_PARAM(z)) {};
private: 
	const Sampler* m_sampler;
};

#endif
