
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
	void setSampler(std::shared_ptr<Sampler> sampler) { mSampler = sampler; }
	std::shared_ptr<Samples> generateSamples(const Eigen::Vector4i& grid);
	virtual void setPosition(float UNUSED_PARAM(x), float UNUSED_PARAM(x), float UNUSED_PARAM(z)) {};
	virtual void setLookAt(float UNUSED_PARAM(x), float UNUSED_PARAM(x), float UNUSED_PARAM(z)) {};
private: 
	std::shared_ptr<Sampler> mSampler;
};

#endif
