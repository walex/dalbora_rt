#ifndef __rhi_pipeline_layout_hpp__
#define __rhi_pipeline_layout_hpp__

#include "rhi_impl.hpp"

class RhiDevice;
class RhiPipelineLayout
	: public ICreateRhiObject<const RhiDevice&, const primitive_topology,
		const resource_format, const resource_format>
	, public RhiImpl<RHI_PIPELINE_LAYOUT> {
public:

	RhiPipelineLayout(RHI_PIPELINE_LAYOUT* handle = nullptr);
	virtual ~RhiPipelineLayout() = default;

	void create(const RhiDevice& device, const primitive_topology topology,
				const resource_format surface_format, const resource_format depth_buffer_format);
	void add_constants_buffer_descriptors(const size_t offset, const size_t count);
	primitive_topology get_topology() const;
	resource_format get_format() const;
	resource_format get_depth_buffer_format() const;
private:
	RHI_PIPELINE_LAYOUT_DESC m_layout_desc;
};

#endif // __rhi_pipeline_layout_hpp__
