#include "rhi_device.hpp"

RhiDevice::RhiDevice(RHI_DEVICE* handle)
	: RhiImpl<RHI_DEVICE>(handle) {}

void RhiDevice::create(const uint32_t adapter_id, const __int64 features) {

	RHI_DEVICE_DESC desc;
	desc.adapter_id = adapter_id;
	desc.features = features;
	this->set_handle(rhi_create_device(&desc));
}