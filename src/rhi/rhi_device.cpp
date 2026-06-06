#include "rhi_device.hpp"
#include "rhi.hpp"

RhiDevice::RhiDevice(RHI_DEVICE* handle)
	: RhiImpl<RHI_DEVICE>(handle) {}

void RhiDevice::create(const uint32_t adapter_id, const __int64 features) {

	RHI_DEVICE_DESC desc;
	desc.adapter_id = adapter_id;
	desc.features = features;
	this->set_handle(rhi_create_device(&desc));

	constant_buffer_slot_id = static_cast<RHI_DEVICE*>(*this)->constant_buffer_slot_start;
	rw_buffer_slot_id = static_cast<RHI_DEVICE*>(*this)->rw_buffer_slot_start;
	read_only_buffer_slot_id = static_cast<RHI_DEVICE*>(*this)->read_only_buffer_slot_start;
	render_target_slot_id = static_cast<RHI_DEVICE*>(*this)->render_target_slot_start;
	depth_buffer_slot_id = static_cast<RHI_DEVICE*>(*this)->depth_buffer_slot_start;
	sampler_slot_id = static_cast<RHI_DEVICE*>(*this)->sampler_slot_start;
}