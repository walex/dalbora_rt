#ifndef __rhi_buffer_hpp__
#define __rhi_buffer_hpp__

#include "rhi_impl.hpp"

class RhiDevice;
class RhiBuffer: public ICreateRhiObject<const RhiDevice&, const size_t,
	const size_t, const resource_format >
	, public RhiImpl<RHI_BUFFER> {

public:
	IMPLEMENT_MOVABLE_CLASS(RhiBuffer);

	RhiBuffer(RHI_BUFFER* handle = nullptr) : RhiImpl<RHI_BUFFER>(handle) {}
	virtual ~RhiBuffer() = default;
	virtual void create(const RhiDevice& device, const size_t length,
		const size_t stride, const resource_format format = resource_format_none) = 0;
	resource_format get_format() const  { return static_cast<RHI_BUFFER*>(*this)->format; }
	resource_type get_type() const { return static_cast<RHI_BUFFER*>(*this)->type; }
	size_t get_length() const { return static_cast<RHI_BUFFER*>(*this)->length; }
	size_t get_stride() const  { return static_cast<RHI_BUFFER*>(*this)->stride; }
};

#endif // __rhi_buffer_hpp__