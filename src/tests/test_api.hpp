#ifndef __test_api_h__
#define __test_api_h__

#include "dalbora_rt_api.hpp"

using test_swap_chain_on_init = std::function<void(RHI_DEVICE& device, RHI_COMMAND_QUEUE& command_queue, RHI_COMMAND_BUFFER& command_buffer)>;
using test_swap_chain_on_before_draw = std::function<void(RHI_RENDER_PASS& render_pass)>;
using test_swap_chain_on_draw = std::function<void(RHI_RENDER_PASS& render_pass, RHI_COMMAND_BUFFER& command_buffer)>;
using test_swap_chain_on_after_draw = std::function<void(RHI_RENDER_PASS& render_pass)>;
using test_swap_chain_on_end = std::function<void(RHI_DEVICE& device)>;


void test_create_window(std::shared_ptr<RHI_WINDOW_CALLBACKS> callbacks = std::shared_ptr<RHI_WINDOW_CALLBACKS>());
void test_swap_chain(test_swap_chain_on_init on_init = nullptr
	, test_swap_chain_on_before_draw on_before_draw = nullptr
	, test_swap_chain_on_draw on_draw = nullptr
	, test_swap_chain_on_after_draw on_after_draw = nullptr
	, test_swap_chain_on_end on_end = nullptr);
void test_raster_triangle(std::function<void(RHI_DEVICE& device)> on_init = nullptr
	, std::function<void(RHI_RENDER_PASS& render_pass)> on_draw = nullptr
	, std::function<void(RHI_DEVICE& device)> on_end = nullptr);
#endif