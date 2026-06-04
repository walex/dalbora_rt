#ifndef __rhi_sampler_hpp__
#define __rhi_sampler_hpp__

#include "rhi_impl.hpp"

class RhiSampler
	: public RhiImpl<RHI_SAMPLER> {

public:	
	RhiSampler(RHI_SAMPLER* handle = nullptr);
	virtual ~RhiSampler() = default;
};

#endif // __rhi_sampler_hpp__
