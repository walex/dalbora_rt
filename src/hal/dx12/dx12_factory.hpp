#ifndef __dx12_factory_hpp__
#define __dx12_factory_hpp__

#include "dx12_hal.hpp"

IDXGIFactory5* dx12_get_factory();
void dx12_destroy_factory();
void dx12_create_factory();

#endif