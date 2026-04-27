#ifndef __dx12_command_buffer_hpp__
#define __dx12_command_buffer_hpp__

#include "dx12_rhi.hpp"

std::unique_ptr<RHI_OBJECT> dx12_create_command_buffer(const RHI_COMMAND_BUFFER_DESC& cb_desc);

#endif