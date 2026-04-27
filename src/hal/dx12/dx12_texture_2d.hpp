#ifndef __dx12_texture_2d_hpp__
#define __dx12_texture_2d_hpp__

#include "dx12_hal.hpp"

std::unique_ptr<HAL_OBJECT> dx12_create_texture_2d(const HAL_TEXTURE_2D_DESC& tex_desc);
#endif