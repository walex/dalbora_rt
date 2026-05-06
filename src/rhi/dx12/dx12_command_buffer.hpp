#ifndef __dx12_command_buffer_hpp__
#define __dx12_command_buffer_hpp__

#include "dx12_rhi.hpp"

std::unique_ptr<RHI_COMMAND_BUFFER> dx12_command_buffer_create(const RHI_COMMAND_BUFFER_DESC& cb_desc);
void dx12_command_buffer_record(RHI_COMMAND_BUFFER& command_buffer,
								fptr_command_buffer_on_record callback);

#endif