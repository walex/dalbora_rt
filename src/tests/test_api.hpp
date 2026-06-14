#ifndef __test_api_h__
#define __test_api_h__

#include "rhi.hpp"
#include "test_api_helpers.hpp"

constexpr rhi_api render_api = rhi_api_dx12;

using fptr_test_on_init = std::function<void(RHI_DEVICE &device, RHI_COMMAND_QUEUE &command_queue, RHI_COMMAND_BUFFER &command_buffer, RHI_SWAP_CHAIN& swap_chain)>;
using fptr_test_on_before_draw = std::function<void(RHI_RENDER_PASS &render_pass)>;
using fptr_test_on_draw = std::function<void(RHI_DEVICE& device, RHI_RENDER_PASS &render_pass, RHI_COMMAND_BUFFER &command_buffer)>;
using fptr_test_on_before_present = std::function<void(RHI_RENDER_PASS &render_pass, RHI_SWAP_CHAIN& swap_chain, RHI_COMMAND_BUFFER& command_buffer)>;
using fptr_test_on_end = std::function<void(RHI_DEVICE &device)>;
using fptr_test_on_layout = std::function<void(RHI_PIPELINE_LAYOUT_DESC&, std::vector<RHI_INPUT_LAYOUT_DESC>&, std::string&, std::string&, size_t&, void**)>;
using fptr_test_on_configure_device = std::function<void(RHI_DEVICE_DESC&)>;

void test_create_window(std::shared_ptr<RHI_WINDOW_CALLBACKS> callbacks = std::shared_ptr<RHI_WINDOW_CALLBACKS>());
void test_swap_chain(fptr_test_on_init on_init = nullptr, 
	fptr_test_on_before_draw on_before_draw = nullptr,
	fptr_test_on_draw on_draw = nullptr,
	fptr_test_on_before_present on_before_present = nullptr, 
	fptr_test_on_end on_end = nullptr,
	fptr_test_on_configure_device on_configure_device = nullptr);
void test_raster_triangle(fptr_test_on_init on_init = nullptr, 
	fptr_test_on_draw on_draw = nullptr, 
	fptr_test_on_end on_end = nullptr, 
	fptr_test_on_layout on_layout = nullptr,
	fptr_test_on_configure_device on_configure_device = nullptr);
void test_raster_textured_triangle(fptr_test_on_init on_init = nullptr, 
	fptr_test_on_draw on_draw = nullptr, 
	fptr_test_on_end on_end = nullptr, 
	fptr_test_on_layout on_layout = nullptr,
	fptr_test_on_configure_device on_configure_device = nullptr);
void test_rt_triangle(fptr_test_on_init on_init = nullptr, 
	fptr_test_on_draw on_draw = nullptr, 
	fptr_test_on_end on_end = nullptr, 
	fptr_test_on_layout on_layout = nullptr,
	fptr_test_on_configure_device on_configure_device = nullptr);

#define TEST_WINDOW
#define TEST_SWAP_CHAIN
#define TEST_RASTER_TRIANGLE
#define TEST_RASTER_TEXTURED_TRIANGLE
#define TEST_RT_TRIANGLE

struct RhiUnitTest {

	RhiWindow window;
	RhiDevice device;
	RhiGraphicsCommandQueue command_queue;
	RhiCommandBuffer command_buffer;
	RhiSwapChain swap_chain;
	RhiRasterRenderPass raster_render_pass;
	RhiPipelineLayout pipeline_layout;
	RhiRasterPipeline raster_pipeline;
	RhiRayTracePipeline ray_trace_pipeline;
	std::string vertex_shader_file;
	std::string pixel_shader_file;
	std::string ray_gen_shader_file;
	std::string miss_shader_file;
	std::string closest_hit_shader_file;
	std::vector<uint8_t> vertices;
	std::vector<uint16_t> indices;
	size_t vertices_stride;
	size_t indices_stride;
};

struct RhiUnitTestCallbacks {
	std::function<void(__int64&)>on_device_config = [](__int64& UNUSED_PARAM(feature_flags)) {};
	std::function<void(RhiUnitTest&)> on_init = [](RhiUnitTest& UNUSED_PARAM(unit_test)) {};
	std::function<void(RhiUnitTest&)> on_draw = [](RhiUnitTest& UNUSED_PARAM(unit_test)) {};
	std::function<void(RhiUnitTest&)> on_end = [](RhiUnitTest& UNUSED_PARAM(unit_test)) {};
};

void test_create_window_obj(RhiUnitTestCallbacks* callbacks = nullptr);
void test_create_swap_chain_obj(RhiUnitTestCallbacks* callbacks = nullptr);
void test_raster_triangle_obj(RhiUnitTestCallbacks* callbacks = nullptr);
void test_raster_textured_triangle_obj(RhiUnitTestCallbacks* callbacks = nullptr);
void test_rt_triangle_obj(RhiUnitTestCallbacks* callbacks = nullptr);
void test_rt_mesh_obj(RhiUnitTestCallbacks* callbacks = nullptr);
#endif
