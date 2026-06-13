#include "test_api.hpp"

#ifdef TEST_SWAP_CHAIN
void test_swap_chain(fptr_test_on_init on_init
	, fptr_test_on_before_draw on_before_draw
	, fptr_test_on_draw on_draw
	, fptr_test_on_before_present on_before_present
	, fptr_test_on_end on_end
    , fptr_test_on_configure_device on_configure_device) {

	std::unique_ptr<RHI_DEVICE> device;
	std::unique_ptr<RHI_COMMAND_QUEUE> command_queue;
	std::unique_ptr<RHI_SWAP_CHAIN> swap_chain;
	std::unique_ptr<RHI_COMMAND_BUFFER> command_buffer;
	std::unique_ptr<RHI_RENDER_PASS> render_pass;
	
	std::shared_ptr<RHI_WINDOW_CALLBACKS> callbacks = std::make_shared<RHI_WINDOW_CALLBACKS>();
	callbacks.get()->on_init = ([&](RHI_WINDOW* const window) {

		RHI_DEVICE_DESC device_desc;
		device_desc.adapter_id = 0;
		device_desc.features = device_features_none;
		if (on_configure_device)
			on_configure_device(device_desc);

		device.reset(rhi_create_device(&device_desc));

		RHI_COMMAND_QUEUE_DESC queue_desc;
		queue_desc.device = device.get();
		command_queue.reset(rhi_command_queue_create_for_render(&queue_desc));

		RHI_COMMAND_BUFFER_DESC command_buffer_desc;
		command_buffer_desc.device = device.get();
		command_buffer_desc.command_queue = command_queue.get();
		command_buffer.reset(rhi_command_buffer_create_for_render(&command_buffer_desc));

		RHI_SWAP_CHAIN_DESC swap_chain_desc;
		swap_chain_desc.device = device.get();
		swap_chain_desc.command_queue = command_queue.get();
		swap_chain_desc.window = window;
		swap_chain_desc.width = 800;
		swap_chain_desc.height = 600;
		swap_chain_desc.allow_tearing = true;
		swap_chain_desc.buffer_count = 3;
		swap_chain_desc.color_format = resource_format_R8G8B8A8_norm;
		swap_chain.reset(rhi_swap_chain_create(&swap_chain_desc));

		RHI_VIEWPORT vp;
		vp.x = 0;
		vp.y = 0;
		vp.width = 800;
		vp.height = 600;
		vp.min_z = 0.0f;
		vp.max_z = 1.0f;

		RHI_RENDER_PASS_DESC render_pass_desc;
		render_pass_desc.device = device.get();
		render_pass.reset(rhi_render_pass_create(&render_pass_desc));
		render_pass->view_port = vp;

		if (on_init)
			on_init(*device, *command_queue, *command_buffer, *swap_chain);
		});

	callbacks.get()->main_loop = ([&](const RHI_WINDOW* UNUSED_PARAM(window)) {

		render_pass->render_target_view = rhi_swap_chain_get_surface(swap_chain.get(), UINT64_MAX);
		
		if (on_before_draw)
			on_before_draw(*render_pass);

		rhi_command_queue_execute(command_queue.get(), true, [&](
			RHI_VOID_PTR UNUSED_PARAM(native_command_queue_impl),
			std::vector<RHI_COMMAND_BUFFER*>* const command_buffer_list) {

				rhi_command_buffer_record(command_buffer.get(),
					[&](RHI_VOID_PTR UNUSED_PARAM(native_command_buffer_impl)) {
						// begin pass
						rhi_render_pass_execute_raster_mode(render_pass.get(), command_buffer.get(), [&] {

							if (on_draw)
								on_draw(*device, *render_pass, *command_buffer);

						});
				});
				command_buffer_list->push_back(command_buffer.get());
		});
		if (on_before_present)
			on_before_present(*render_pass, *swap_chain, *command_buffer);

		// present
		rhi_swap_chain_present(swap_chain.get());
		print_fps();
	});
	test_create_window(callbacks);

	{
		if (on_end)
			on_end(*device);

		// release objects in order
		swap_chain.reset();
		command_buffer.reset();
		command_queue.reset();
		render_pass.reset();
		swap_chain.reset();
		device.reset();
	}
	return;

}

void test_create_swap_chain_obj(RhiUnitTestCallbacks* callbacks) {
	
	
	RHI_VIEWPORT vp;
	vp.x = 0;
	vp.y = 0;
	vp.width = 800;
	vp.height = 600;
	vp.min_z = 0.0f;
	vp.max_z = 1.0f;

	RhiUnitTestCallbacks unit_test_callbacks;
	unit_test_callbacks.on_init = ([&](RhiUnitTest& unit_test) {
		
		RhiWindow& window = unit_test.window;
		RhiDevice& device = unit_test.device;
		RhiGraphicsCommandQueue& command_queue = unit_test.command_queue;
		RhiCommandBuffer& command_buffer = unit_test.command_buffer;
		RhiSwapChain& swap_chain = unit_test.swap_chain;
		RhiRenderPass& render_pass = unit_test.raster_render_pass;
		
		device.create(0, device_features_none);
		command_queue.create(device);
		command_buffer.create(device, command_queue);
		swap_chain.create(window, device, command_queue);
		render_pass.create(device);
		if (callbacks)
			callbacks->on_init(unit_test);
	});

	unit_test_callbacks.on_draw = ([&](RhiUnitTest& unit_test) {

		RhiWindow& window = unit_test.window;
		RhiDevice& device = unit_test.device;
		RhiGraphicsCommandQueue& command_queue = unit_test.command_queue;
		RhiCommandBuffer& command_buffer = unit_test.command_buffer;
		RhiSwapChain& swap_chain = unit_test.swap_chain;
		RhiRenderPass& render_pass = unit_test.raster_render_pass;

		command_buffer.record([&] {
			
			RhiView render_target_view = swap_chain.get_next_render_target();
			render_pass.set_view_port(vp);
			render_pass.set_render_target(render_target_view);
			render_pass.render(command_buffer, [&](RhiCommandBuffer& command_buffer) {
				
				if (callbacks)
					callbacks->on_draw(unit_test);
			});
		});

		command_queue.sync_exec([&](RhiCommandQueueBufferList& list) {
			
			list.add_command_buffer(command_buffer);
		});
		swap_chain.present();
	});

	unit_test_callbacks.on_end = ([&](RhiUnitTest& unit_test) {

	});

	test_create_window_obj(&unit_test_callbacks);
}
#endif