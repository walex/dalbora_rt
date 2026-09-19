#ifndef __rhi_device_hpp__
#define __rhi_device_hpp__

#include "rhi_impl.hpp"

class RhiDevice
	: public ICreateRhiObject<const RHI_DEVICE_DESC&>
	, public RhiImpl<RHI_DEVICE> {

public:
	IMPLEMENT_MOVABLE_CLASS(RhiDevice);
	RhiDevice(RHI_DEVICE* handle = nullptr);
	virtual ~RhiDevice() = default;
	void create(const RHI_DEVICE_DESC& desc) override;

	
private:
	
};

#endif // __rhi_device_hpp__