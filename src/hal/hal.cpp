#include "hal.hpp"
#include <exception>

#include "dx12_hal.hpp"

static device_type g_device_type = device_type_none;

void hal_init(device_type dt) {

	hal_end();

	if (dt == device_type_dx12) {
		dx12_hal_init();
	}
	else {
		throw std::exception("Unsupported device type");
	}
	g_device_type = dt;
}

void hal_end() {

	dx12_hal_end();
	g_device_type = device_type_none;
}