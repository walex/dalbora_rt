#include "rhi_sampler.hpp"
#include "rhi.hpp"

RhiSampler::RhiSampler(RHI_SAMPLER* handle) : RhiImpl<RHI_SAMPLER>(handle) {}

void RhiSampler::create(const RhiDevice& device, const RhiMemoryDescriptor* memory_descriptor) {
	
	RHI_SAMPLER_DESC desc;
	desc.device = device;
	desc.memory_descriptor_slot = *memory_descriptor;
	this->set_handle(rhi_sampler_create(&desc));
	m_memory_descriptor.reset(memory_descriptor);
}