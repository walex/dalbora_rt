#include "test_api.hpp"

std::unique_ptr<RhiMemoryTable> buffers_memory_table;
std::unique_ptr<RhiMemoryTable> rtv_memory_table;
std::unique_ptr<RhiMemoryTable> dsv_memory_table;
std::unique_ptr<RhiMemoryTable> samplers_memory_table;

RhiMemoryTable& get_buffers_memory_table() {
	return *buffers_memory_table;
}
RhiMemoryTable& get_rtv_memory_table() {
	return *rtv_memory_table;
}
RhiMemoryTable& get_dsv_memory_table() {
	return *dsv_memory_table;
}
RhiMemoryTable& get_samplers_memory_table() {
	return *samplers_memory_table;
}

void memory_table_init(RHI_DEVICE* device) {

	RhiDevice device_obj(device);

	std::vector<size_t> slot_group_start_indices = {0, 100, 200};
	buffers_memory_table = std::make_unique<RhiMemoryTable>(device_obj, memory_descriptor_type_buffer, BUFFERS_DESCRIPTORS_COUNT, slot_group_start_indices);
	samplers_memory_table = std::make_unique<RhiMemoryTable>(device_obj, memory_descriptor_type_sampler, SAMPLER_DESCRIPTORS_COUNT);
	rtv_memory_table = std::make_unique<RhiMemoryTable>(device_obj, memory_descriptor_type_dx_rtv, RTV_HEAP_DESCRIPTORS_COUNT);
	dsv_memory_table = std::make_unique<RhiMemoryTable>(device_obj, memory_descriptor_type_dx_dsv, DSV_DESCRIPTORS_COUNT);
}

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
	std::vector<std::unique_ptr<RHI_VIEW>> views;
	std::vector<std::unique_ptr<RHI_MEMORY_DESCRIPTOR_SLOT>> views_slots;

	std::shared_ptr<RHI_WINDOW_CALLBACKS> callbacks = std::make_shared<RHI_WINDOW_CALLBACKS>();

	callbacks.get()->on_init = ([&](RHI_WINDOW* const window) {

		RHI_DEVICE_DESC device_desc;
		device_desc.adapter_id = -1;
		device_desc.features = device_features_none;
		if (on_configure_device)
			on_configure_device(device_desc);

		device.reset(rhi_create_device(&device_desc));

		memory_table_init(device.get());

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
		swap_chain_desc.disable_vsync = true;
		swap_chain_desc.buffer_count = 3;
		swap_chain_desc.color_format = resource_format_R8G8B8A8_norm;
		swap_chain.reset(rhi_swap_chain_create(&swap_chain_desc));

		for (size_t i = 0; i < swap_chain_desc.buffer_count; i++) {

			views_slots.emplace_back(get_rtv_memory_table().next_descriptor().release());
			RHI_VIEW* view_ptr = rhi_swap_chain_create_view(device.get(), swap_chain.get(), views_slots.back().get(), swap_chain_desc.color_format, i);
			views.push_back(std::unique_ptr<RHI_VIEW>(view_ptr));
		}

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

	callbacks.get()->on_idle = ([&](const RHI_WINDOW* UNUSED_PARAM(window)) {

		size_t i = rhi_swap_chain_get_current_buffer_id(swap_chain.get());
		render_pass->render_target_view = views[i].get();
		
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

	__int64 device_features = device_features_none;
	RhiUnitTestCallbacks unit_test_callbacks;

	unit_test_callbacks.on_init = ([&](RhiUnitTest& unit_test) {
		
		RhiWindow& window = unit_test.window;
		RhiDevice& device = unit_test.device;
		RhiGraphicsCommandQueue& command_queue = unit_test.command_queue;
		RhiCommandBuffer& command_buffer = unit_test.command_buffer;
		RhiSwapChain& swap_chain = unit_test.swap_chain;
		RhiRenderPass& render_pass = unit_test.raster_render_pass;
		
		RHI_DEVICE_DESC device_desc;
		device_desc.adapter_id = -1;
		device_desc.shader_model = hlsl_shader_model_6_8;

		if(callbacks)
			callbacks->on_device_config(device_desc);

		if (callbacks)
			callbacks->on_memory_descriptor_config(unit_test.read_only_shader_registers_count, unit_test.rw_shader_registers_count,
				unit_test.constant_shader_registers_count);

		device.create(device_desc);

		std::vector<size_t> slot_group_start_indices = { 0, 100, 200 };
		unit_test.resources_memory_descriptors = std::make_unique<RhiMemoryTable>(device, memory_descriptor_type_buffer,
			unit_test.read_only_shader_registers_count + unit_test.rw_shader_registers_count + unit_test.constant_shader_registers_count,
			slot_group_start_indices);

		unit_test.rtv_memory_descriptors = std::make_unique<RhiMemoryTable>(device, memory_descriptor_type_dx_rtv, RTV_HEAP_DESCRIPTORS_COUNT);
		unit_test.dsv_memory_descriptors = std::make_unique<RhiMemoryTable>(device, memory_descriptor_type_dx_dsv, DSV_DESCRIPTORS_COUNT);

		command_queue.create(device);
		command_buffer.create(device, command_queue);
		command_buffer.set_resources_memory_descriptor(*unit_test.resources_memory_descriptors);
		swap_chain.create(window, device, command_queue);
		swap_chain.create_views(device, *unit_test.rtv_memory_descriptors);
		render_pass.create(device);
		if (callbacks)
			callbacks->on_init(unit_test);
	});

	unit_test_callbacks.on_draw = ([&](RhiUnitTest& unit_test) {

		RhiGraphicsCommandQueue& command_queue = unit_test.command_queue;
		RhiCommandBuffer& command_buffer = unit_test.command_buffer;
		RhiSwapChain& swap_chain = unit_test.swap_chain;
		RhiRenderPass& render_pass = unit_test.raster_render_pass;

		command_buffer.record([&] {
			
			RhiView render_target_view = swap_chain.get_next_render_target();
			render_pass.set_view_port(vp);
			render_pass.set_render_target(render_target_view);
			render_pass.render(command_buffer, [&](RhiCommandBuffer& UNUSED_PARAM(command_buffer)) {
				
				if (callbacks)
					callbacks->on_draw(unit_test);
			});
		});

		command_queue.sync_exec([&](RhiCommandQueueBufferList& list) {
			
			list.add_command_buffer(command_buffer);
		});
		swap_chain.present();
	});

	unit_test_callbacks.on_end = ([&](RhiUnitTest& UNUSED_PARAM(unit_test)) {

	});

	
	test_create_window_obj(&unit_test_callbacks);
}
#endif