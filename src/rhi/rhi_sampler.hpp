#ifndef __rhi_sampler_hpp__
#define __rhi_sampler_hpp__

#include "rhi_impl.hpp"

class RhiDevice;
class RhiMemoryDescriptorSlot;
class RhiSampler
	: public RhiImpl<RHI_SAMPLER>
    , public ICreateRhiObject<const RhiDevice&, const RhiMemoryDescriptorSlot*> {

public:	
	RhiSampler(RHI_SAMPLER* handle = nullptr);
	virtual ~RhiSampler() = default;
	void create(const RhiDevice& device, const RhiMemoryDescriptorSlot* memory_descriptor) override;
private:
	std::unique_ptr<const RhiMemoryDescriptorSlot> m_memory_descriptor;
};

#endif // __rhi_sampler_hpp__
