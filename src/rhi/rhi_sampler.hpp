#ifndef __rhi_sampler_hpp__
#define __rhi_sampler_hpp__

#include "rhi_impl.hpp"

class RhiDevice;
class RhiSampler
	: public RhiImpl<RHI_SAMPLER>
    , public ICreateRhiObject<const RhiDevice&> {

public:	
	RhiSampler(RHI_SAMPLER* handle = nullptr);
	virtual ~RhiSampler() = default;
	void create(const RhiDevice& device) override;
};

#endif // __rhi_sampler_hpp__
