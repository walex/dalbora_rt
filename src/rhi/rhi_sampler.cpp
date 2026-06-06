#include "rhi_sampler.hpp"
#include "rhi.hpp"

RhiSampler::RhiSampler(RHI_SAMPLER* handle) : RhiImpl<RHI_SAMPLER>(handle) {}

void RhiSampler::create(const RhiDevice& device) {
	
	RHI_RT_SAMPLER_DESC desc;
	desc.device = device;
	this->set_handle(rhi_sampler_create(&desc));
}