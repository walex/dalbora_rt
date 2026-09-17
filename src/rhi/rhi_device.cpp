#include "rhi_device.hpp"

RhiDevice::RhiDevice(RHI_DEVICE* handle)
	: RhiImpl<RHI_DEVICE>(handle) {}

void RhiDevice::create(const RHI_DEVICE_DESC& desc) {
	this->set_handle(rhi_create_device(&desc));
}