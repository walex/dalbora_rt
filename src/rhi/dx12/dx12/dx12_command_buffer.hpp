#ifndef __dx12_command_buffer_hpp__
#define __dx12_command_buffer_hpp__

#include "dx12_rhi.hpp"

std::unique_ptr<RHI_OBJECT> dx12_command_buffer_create(const RHI_COMMAND_BUFFER_DESC& cb_desc);
void dx12_command_buffer_begin_record(RHI_OBJECT& command_buffer);
void dx12_command_buffer_end_record(RHI_OBJECT& command_buffer);

#endif