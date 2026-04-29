#include "test_api.hpp"
#include "dx12_rhi.hpp"

void test_swap_chain() {

	std::unique_ptr<RHI_OBJECT> device;
	std::unique_ptr<RHI_OBJECT> queue;
	std::unique_ptr<RHI_OBJECT> swap_chain;

	RHI_WINDOW_CALLBACKS callbacks;
	callbacks.on_init = ([&device, &queue, &swap_chain] (RHI_OBJECT& window_handle) {

		RHI_DEVICE_DESC device_desc;
		device_desc.adapter_id = 0;
		device_desc.features = device_features_raytracing;
		device = rhi_create_device(device_desc);

		RHI_COMMAND_QUEUE_DESC queue_desc;
		queue_desc.device = device.get();
		queue = rhi_create_graphics_command_queue(queue_desc);

		RHI_SWAP_CHAIN_DESC swap_chain_desc;
		swap_chain_desc.width = 800;
		swap_chain_desc.height = 600;
		swap_chain_desc.allow_tearing = false;
		swap_chain_desc.buffer_count = 2;
		swap_chain_desc.color_format = resource_format_R8G8B8A8;
		swap_chain_desc.device = queue_desc.device;
		swap_chain_desc.command_queue = queue.get();
		swap_chain_desc.window = &window_handle;
		swap_chain = rhi_create_swap_chain(swap_chain_desc);
		});

	callbacks.main_loop = ([&swap_chain] {

		//rhi_begin_render_pass(render_traget);

		rhi_swap_chain_present(*swap_chain.get());
	});
	test_create_window(&callbacks);
}

