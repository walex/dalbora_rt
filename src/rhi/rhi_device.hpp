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

	uint64_t next_constant_buffer_slot_id() const  {
		printf("next_constant_buffer_slot_id: %llu\n", m_constant_buffer_slot.current);
		return m_constant_buffer_slot.current++;
	}

	uint64_t next_rw_buffer_slot_id() const  {
		printf("next_rw_buffer_slot_id: %llu\n", m_rw_buffer_slot.current);
		return m_rw_buffer_slot.current++; 
	}
	
	uint64_t next_read_only_buffer_slot_id() const { 
		printf("next_read_only_buffer_slot_id: %llu\n", m_read_only_buffer_slot.current);
		return m_read_only_buffer_slot.current++; 
	}

	uint64_t next_render_target_slot_id() const  {
		printf("next_render_target_slot_id: %llu\n", m_render_target_slot.current);
		return m_render_target_slot.current++; 
	}

	uint64_t next_depth_buffer_slot_id() const  { 
		printf("next_depth_buffer_slot_id: %llu\n", m_depth_buffer_slot.current);
		return m_depth_buffer_slot.current++; 
	}

	uint64_t next_sampler_slot_id() const  { 
		printf("next_sampler_slot_id: %llu\n", m_sampler_slot.current);
		return m_sampler_slot.current++; 
	}
private:
	mutable RhiShaderRegisterSlots m_constant_buffer_slot = {};
	mutable RhiShaderRegisterSlots m_rw_buffer_slot = {};
	mutable RhiShaderRegisterSlots m_read_only_buffer_slot = {};
	mutable RhiShaderRegisterSlots m_render_target_slot = {};
	mutable RhiShaderRegisterSlots m_depth_buffer_slot = {};
	mutable RhiShaderRegisterSlots m_sampler_slot = {};
};

#endif // __rhi_device_hpp__