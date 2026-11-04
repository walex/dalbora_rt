#include "rhi_api.hpp"
#include <exception>

#include "window_handle.hpp"
#include "dx12_rhi.hpp"
#include "vk_rhi.hpp"

static rhi_device_type g_device_type = rhi_device_type_none;

void rhi_init(rhi_device_type dt) {

	rhi_end();

	// window
	rhi_create_window = &window_handle_create;
	rhi_window_main_loop = &window_handle_main_loop;

	switch (dt) {
	case rhi_device_type_dx12:
		dx12_rhi_init();
		break;
	case rhi_device_type_vulkan:
		vk_rhi_init();
		break;
	default:
		throw std::exception("Unsupported device type");
	}
	g_device_type = dt;
}

void rhi_end() {

	switch (g_device_type) {
	case rhi_device_type_none:
		break;
	case rhi_device_type_dx12:
		dx12_rhi_end();
		break;
	case rhi_device_type_vulkan:
		vk_rhi_end();
		break;
	default:
		throw std::exception("Unsupported device type");
	}
	g_device_type = rhi_device_type_none;
}

rhi_device_type rhi_get_device_type() { return g_device_type; }

RHI_APP_INSTANCE rhi_get_app_instance() {

	switch (g_device_type) {
	case rhi_device_type_none:
		return nullptr;
	case rhi_device_type_dx12:
		return dx12_rhi_get_app_instance();
	case rhi_device_type_vulkan:
		return vk_rhi_get_app_instance();
	default:
		throw std::exception("Unsupported device type");
	}
}