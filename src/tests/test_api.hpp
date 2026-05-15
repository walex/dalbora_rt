#ifndef __test_api_h__
#define __test_api_h__

#include "dalbora_rt_api.hpp"
#include "test_api_helpers.hpp"

constexpr rhi_api render_api = rhi_api_dx12;

using fptr_test_on_init = std::function<void(RHI_DEVICE &device, RHI_COMMAND_QUEUE &command_queue, RHI_COMMAND_BUFFER &command_buffer)>;
using fptr_test_on_before_draw = std::function<void(RHI_RENDER_PASS &render_pass)>;
using fptr_test_on_draw = std::function<void(RHI_RENDER_PASS &render_pass, RHI_COMMAND_BUFFER &command_buffer)>;
using fptr_test_on_after_draw = std::function<void(RHI_RENDER_PASS &render_pass)>;
using fptr_test_on_end = std::function<void(RHI_DEVICE &device)>;
using fptr_test_on_layout = std::function<void(std::vector<RHI_DESCRIPTOR_DESC>&, std::vector<RHI_INPUT_LAYOUT_DESC>&, std::string&, std::string&, size_t&, void**)>;

void test_create_window(std::shared_ptr<RHI_WINDOW_CALLBACKS> callbacks = std::shared_ptr<RHI_WINDOW_CALLBACKS>());
void test_swap_chain(fptr_test_on_init on_init = nullptr, fptr_test_on_before_draw on_before_draw = nullptr, fptr_test_on_draw on_draw = nullptr, fptr_test_on_after_draw on_after_draw = nullptr, fptr_test_on_end on_end = nullptr);
void test_raster_triangle(fptr_test_on_init on_init = nullptr, fptr_test_on_draw on_draw = nullptr, fptr_test_on_end on_end = nullptr, fptr_test_on_layout on_layout = nullptr);
void test_raster_textured_triangle(fptr_test_on_init on_init = nullptr, fptr_test_on_draw on_draw = nullptr, fptr_test_on_end on_end = nullptr, fptr_test_on_layout on_layout = nullptr);
void test_rt_triangle(fptr_test_on_init on_init = nullptr, fptr_test_on_draw on_draw = nullptr, fptr_test_on_end on_end = nullptr, fptr_test_on_layout on_layout = nullptr);

#endif
