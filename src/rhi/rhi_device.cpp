#include "rhi_device.hpp"
#include "rhi.hpp"

RhiDevice::RhiDevice(RHI_DEVICE* handle)
	: RhiImpl<RHI_DEVICE>(handle) {}

void RhiDevice::create(const RHI_DEVICE_DESC& desc) {

	this->set_handle(rhi_create_device(&desc));

	memcpy(&m_constant_buffer_slot, &static_cast<RHI_DEVICE*>(*this)->constant_buffer_slot, sizeof(RhiShaderRegisterSlots));
	memcpy(&m_rw_buffer_slot, &static_cast<RHI_DEVICE*>(*this)->rw_buffer_slot, sizeof(RhiShaderRegisterSlots));
	memcpy(&m_read_only_buffer_slot, &static_cast<RHI_DEVICE*>(*this)->read_only_buffer_slot, sizeof(RhiShaderRegisterSlots));
	memcpy(&m_render_target_slot, &static_cast<RHI_DEVICE*>(*this)->render_target_slot, sizeof(RhiShaderRegisterSlots));
	memcpy(&m_depth_buffer_slot, &static_cast<RHI_DEVICE*>(*this)->depth_buffer_slot, sizeof(RhiShaderRegisterSlots));
	memcpy(&m_sampler_slot, &static_cast<RHI_DEVICE*>(*this)->sampler_slot, sizeof(RhiShaderRegisterSlots));
}