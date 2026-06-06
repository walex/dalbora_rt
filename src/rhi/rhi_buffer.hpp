#ifndef __rhi_buffer_hpp__
#define __rhi_buffer_hpp__

#include "rhi_impl.hpp"

class RhiDevice;
class RhiBuffer: public ICreateRhiObject<const RhiDevice&, const size_t,
	const size_t, const resource_format >
	, public RhiImpl<RHI_BUFFER> {

public:
	RhiBuffer(RHI_BUFFER* handle) : RhiImpl<RHI_BUFFER>(handle) {}
	virtual ~RhiBuffer() = default;
	virtual void create(const RhiDevice& device, const size_t length,
		const size_t stride, const resource_format format = resource_format_none) = 0;
};

#endif // __rhi_buffer_hpp__