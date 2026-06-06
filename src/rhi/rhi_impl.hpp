#ifndef __rhi_impl_hpp__
#define __rhi_impl_hpp__

#include "rhi_c.h"
#include "rhi_object.hpp"

template<typename T>
class RhiImpl: public RhiObject {
	
public:
	RhiImpl(RhiImpl&&) noexcept = default;
	RhiImpl& operator=(RhiImpl&&) noexcept = default;
	RhiImpl(const RhiImpl&) = delete;
	RhiImpl& operator=(const RhiImpl&) = delete;

	virtual ~RhiImpl() {
		if (m_owner == false)
			m_handle.release();
	};
	operator T* () const { return m_handle.get(); }
	void set_handle(T* handle) {
		if (m_owner == false)
			m_handle.release();
		m_handle.reset(handle);
		m_owner = true;
	}
protected:
	RhiImpl(T* no_owned_handle) {
		if (no_owned_handle) {
			m_handle.reset(no_owned_handle);
			m_owner = false;
		}
	}
private:
	bool m_owner = true;
	std::unique_ptr<T> m_handle;
};

#endif // __rhi_class_hpp__
