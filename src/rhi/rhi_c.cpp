#include "rhi_c.h"
#include <exception>

#include "dx12_rhi.hpp"

static device_type g_device_type = device_type_none;

void rhi_init(device_type dt) {

	rhi_end();

	if (dt == device_type_dx12) {
		dx12_rhi_init();
	}
	else {
		throw std::exception("Unsupported device type");
	}
	g_device_type = dt;
}

void rhi_end() {

	dx12_rhi_end();
	g_device_type = device_type_none;
}