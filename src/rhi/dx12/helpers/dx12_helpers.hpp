#ifndef __dx12_helpers_hpp__
#define __dx12_helpers_hpp__

#include "platform.hpp"
#include "rhi.hpp"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxcapi.h>
#include <wrl.h>

resource_format dx12_helpers_resource_format_from_dxgi_format(const DXGI_FORMAT format);

#endif