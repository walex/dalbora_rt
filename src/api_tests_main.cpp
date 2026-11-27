#include "dalbora_rt_api.hpp"
void test_raster_triangle();
void test_raster_textured_triangle();
void test_rt_triangle();
void test_raster_triangle_obj();
void test_raster_textured_triangle_obj();

void test_api()
{
	// init rhi
	rhi_init(rhi_device_type_dx12);
	//rhi_init(rhi_device_type_vulkan);
	
	// simple platform windows creation
	//test_create_window();

	// swap chain
	//test_swap_chain();

	// raster triangle
	//test_raster_triangle();

	// raster textured triangle
    // test_raster_textured_triangle();

	// rt triangle
	test_rt_triangle();

	// end rhi
	rhi_end();
}


void test_objects() {

	// init rhi
	rhi_init(rhi_device_type_dx12);
	//rhi_init(rhi_device_type_vulkan);

	//test_create_window_obj();

	// test_create_swap_chain_obj();

	// test_raster_triangle_obj();

	test_raster_textured_triangle_obj();

	//test_rt_triangle_obj();
	
	//test_rt_mesh_obj();

	//test_rt_scene_obj();

	// end rhi
	rhi_end();
}

int main(int UNUSED_PARAM(argc), char *UNUSED_PARAM(argv[]))
{

	//test_api();
	test_objects();

	return 0;
}