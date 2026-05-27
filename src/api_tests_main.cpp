#include "dalbora_rt_api.hpp"
#include "test_api.hpp"

int main(int UNUSED_PARAM(argc), char *UNUSED_PARAM(argv[]))
{

	// init rhi
	rhi_init(device_type_dx12);

	// simple platform windows creation
	// test_create_window();

	// swap chain
	// test_swap_chain();

	// raster triangle
	// test_raster_triangle();

	// raster textured triangle
	 test_raster_textured_triangle();

	// rt triangle
	//test_rt_triangle();

	// end rhi
	rhi_end();

	return 0;
}