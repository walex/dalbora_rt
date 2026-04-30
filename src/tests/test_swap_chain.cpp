#include "test_api.hpp"
#include "dx12_rhi.hpp"

void test_swap_chain() {

	std::unique_ptr<RHI_OBJECT> device;
	std::unique_ptr<RHI_OBJECT> queue;
	std::unique_ptr<RHI_OBJECT> swap_chain;
	std::unique_ptr<RHI_OBJECT> command_buffer;
	std::vector<std::unique_ptr<RHI_OBJECT>> render_passes;

	RHI_WINDOW_CALLBACKS callbacks;
	callbacks.on_init = ([&] (RHI_OBJECT& window_handle) {

		RHI_DEVICE_DESC device_desc;
		device_desc.adapter_id = 0;
		device_desc.features = device_features_raytracing;
		device = rhi_create_device(device_desc);

		RHI_COMMAND_QUEUE_DESC queue_desc(*device);
		queue = rhi_command_queue_create_for_render(queue_desc);

		RHI_COMMAND_BUFFER_DESC command_buffer_desc(*device.get(), *queue.get());
		command_buffer = rhi_command_buffer_create(command_buffer_desc);

		RHI_SWAP_CHAIN_DESC swap_chain_desc(*device.get(), *queue.get(), window_handle);
		swap_chain_desc.width = 800;
		swap_chain_desc.height = 600;
		swap_chain_desc.allow_tearing = false;
		swap_chain_desc.buffer_count = kSwapChainBufferCount;
		swap_chain_desc.color_format = resource_format_R8G8B8A8;
		swap_chain = rhi_swap_chain_create(swap_chain_desc);

		for (int i = 0; i < kSwapChainBufferCount; i++) {

			auto back_buffer = rhi_swap_chain_get_surface(*swap_chain.get(), i);
			RHI_RENDER_PASS_DESC render_pass_desc(*device.get(), std::move(back_buffer));
			render_pass_desc.buffer_index = i;
			auto render_pass = rhi_render_pass_create(render_pass_desc);
			render_passes.emplace_back(render_pass.release());
		}
		});

	callbacks.main_loop = ([&] {

		unsigned int id = rhi_swap_chain_get_current_buffer_id(*swap_chain.get());
		auto& render_pass = render_passes[id];
		rhi_render_pass_begin(*render_pass.get(), *command_buffer.get());
		rhi_render_pass_end(*render_pass.get(), *queue.get(), *command_buffer.get());
		rhi_swap_chain_present(*swap_chain.get());
	});
	test_create_window(&callbacks);
}

