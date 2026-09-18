#ifndef __rhi_sampler_hpp__
#define __rhi_sampler_hpp__

#include "rhi_impl.hpp"

class RhiDevice;
class RhiSampler
	: public RhiImpl<RHI_SAMPLER>
    , public ICreateRhiObject<const RhiDevice&, RHI_MEMORY_DESCRIPTOR_SLOT*> {

public:	
	RhiSampler(RHI_SAMPLER* handle = nullptr);
	virtual ~RhiSampler() = default;
	void create(const RhiDevice& device, RHI_MEMORY_DESCRIPTOR_SLOT* memory_descriptor_slot) override;
private:
	std::unique_ptr<RHI_MEMORY_DESCRIPTOR_SLOT> m_memory_descriptor;
};

#endif // __rhi_sampler_hpp__
