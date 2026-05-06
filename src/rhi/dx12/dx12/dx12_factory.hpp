#ifndef __dx12_factory_hpp__
#define __dx12_factory_hpp__

#include "dx12_rhi.hpp"

IDXGIFactory5* dx12_factory_get();
void dx12_factory_destroy();
void dx12_factory_create();

#endif