#ifndef __rhi_device_hpp__
#define __rhi_device_hpp__

#include "rhi_impl.hpp"

class RhiDevice
	: public ICreateRhiObject<const RHI_DEVICE_DESC&>
	, public RhiImpl<RHI_DEVICE> {

public:
	RhiDevice(RHI_DEVICE* handle = nullptr);
	virtual ~RhiDevice() = default;
	void create(const RHI_DEVICE_DESC& desc) override;
	uint64_t next_constant_buffer_slot_id() const  { return constant_buffer_slot_id++; }
	uint64_t next_rw_buffer_slot_id() const  { return rw_buffer_slot_id++; }
	uint64_t next_read_only_buffer_slot_id() const { return read_only_buffer_slot_id++; }
	uint64_t next_render_target_slot_id() const  { return render_target_slot_id++; }
	uint64_t next_depth_buffer_slot_id() const  { return depth_buffer_slot_id++; }
	uint64_t next_sampler_slot_id() const  { return sampler_slot_id++; }
private:
	mutable uint64_t constant_buffer_slot_id = 0;
	mutable uint64_t rw_buffer_slot_id = 0;
	mutable uint64_t read_only_buffer_slot_id = 0;
	mutable uint64_t render_target_slot_id = 0;
	mutable uint64_t depth_buffer_slot_id = 0;
	mutable uint64_t sampler_slot_id = 0;
};

#endif // __rhi_device_hpp__