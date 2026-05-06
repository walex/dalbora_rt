#include "test_api.hpp"
#include "rhi.hpp"


void test_swap_chain(test_swap_chain_on_init on_init
	, test_swap_chain_on_before_draw on_before_draw
	, test_swap_chain_on_draw on_draw
	, test_swap_chain_on_after_draw on_after_draw
	, test_swap_chain_on_end on_end) {

	std::unique_ptr<RHI_DEVICE> device;
	std::unique_ptr<RHI_COMMAND_QUEUE> command_queue;
	std::unique_ptr<RHI_SWAP_CHAIN> swap_chain;
	std::unique_ptr<RHI_COMMAND_BUFFER> command_buffer;
	std::vector<std::unique_ptr<RHI_RENDER_PASS>> render_passes;
	// memory pool for image views implemented only for dx12
	std::unique_ptr<RHI_DESCRIPTOR_POOL> rt_descriptor_pool;
	
	std::shared_ptr<RHI_WINDOW_CALLBACKS> callbacks = std::make_shared<RHI_WINDOW_CALLBACKS>();
	callbacks.get()->on_init = ([&] (RHI_WINDOW& window) {

		RHI_DEVICE_DESC device_desc;
		device_desc.adapter_id = 0;
		device_desc.features = device_features_raytracing;
		device = rhi_create_device(device_desc);

		// applies only to dx12
		RHI_DESCRIPTOR_POOL_DESC dp_desc(*device);
		dp_desc.resource_type = resource_type_render_target;
		dp_desc.slot_count = kImageViewsCount;
		rt_descriptor_pool = rhi_descriptor_pool_create(dp_desc);

		RHI_COMMAND_QUEUE_DESC queue_desc(*device);
		command_queue = rhi_command_queue_create_for_render(queue_desc);
		
		RHI_COMMAND_BUFFER_DESC command_buffer_desc(*device, *command_queue);
		command_buffer = rhi_command_buffer_create(command_buffer_desc);

		RHI_SWAP_CHAIN_DESC swap_chain_desc(*device, *command_queue, window);
		swap_chain_desc.width = 800;
		swap_chain_desc.height = 600;
		swap_chain_desc.allow_tearing = false;
		swap_chain_desc.buffer_count = kSwapChainBufferCount;
		swap_chain_desc.color_format = resource_format_R8G8B8A8_norm;
		swap_chain = rhi_swap_chain_create(swap_chain_desc);

		for (int i = 0; i < kSwapChainBufferCount; i++) {

			auto back_buffer = rhi_swap_chain_get_surface(*swap_chain, i);
			RHI_RENDER_PASS_DESC render_pass_desc(*device, *rt_descriptor_pool, back_buffer);
			render_pass_desc.buffer_index = i;
			render_pass_desc.synchronized = true;
			auto render_pass = rhi_render_pass_create(render_pass_desc);
			render_passes.emplace_back(render_pass.release());
		}
		if (on_init)
			on_init(*device, *command_queue, *command_buffer);
		});

	callbacks.get()->main_loop = ([&](RHI_WINDOW& UNUSED_PARAM(window)) {

		// get current back buffer
		unsigned int id = rhi_swap_chain_get_current_buffer_id(*swap_chain);
		// get associated render pass
		auto& render_pass = render_passes[id];
		
		if (on_before_draw)
			on_before_draw(*render_pass);

		rhi_command_queue_execute(*command_queue, true, [&](RHI_VOID_PTR UNUSED_PARAM(native_command_queue_impl),
			std::vector<RHI_COMMAND_BUFFER*>& command_buffer_list) {

				rhi_command_buffer_record(*command_buffer,
					[&](RHI_VOID_PTR UNUSED_PARAM(native_command_buffer_impl)) {
						// begin pass
						rhi_render_pass_execute(*render_pass, *command_buffer, [&] {

							if (on_draw)
								on_draw(*render_pass);

						});
				});
				command_buffer_list.push_back(command_buffer.get());
		});
		if (on_after_draw)
			on_after_draw(*render_pass);

		// present
		rhi_swap_chain_present(*swap_chain);
	});
	test_create_window(callbacks);

	{
		if (on_end)
			on_end(*device);

		// release objects in order
		swap_chain.reset();
		command_buffer.reset();
		command_queue.reset();
		for (auto& rp : render_passes)
			rp.reset();
		swap_chain.reset();
		device.reset();
	}
	return;

}

