#ifndef __rhi_device_hpp__
#define __rhi_device_hpp__

#include "rhi_impl.hpp"

class RhiDevice
	: public ICreateRhiObject<const uint32_t, const __int64> 
	, public RhiImpl<RHI_DEVICE> {

public:
	RhiDevice(RHI_DEVICE* handle = nullptr);
	virtual ~RhiDevice() = default;
	void create(const uint32_t adapter_id, const __int64 features) override;
};

#endif // __rhi_device_hpp__