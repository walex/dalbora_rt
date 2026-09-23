#ifndef __rhi_impl_hpp__
#define __rhi_impl_hpp__

#include "rhi_api.hpp"
#include "rhi_object.hpp"

template<typename T>
class RhiImpl: public RhiObject {
	
public:
	IMPLEMENT_MOVABLE_CLASS(RhiImpl);

	virtual ~RhiImpl() {
		if (m_owner == false)
			m_handle.release();
	};
	operator T* () const { return m_handle.get(); }
	bool empty() const { return m_handle == nullptr; }

	void set_handle(T* handle) {
		if (m_owner == false)
			m_handle.release();
		m_handle.reset(handle);
		m_owner = true;
	}

protected:
	RhiImpl(T* handle, const bool ownership = false) {
		if (handle) {
			m_handle.reset(handle);
			m_owner = ownership;
		}
	}
	
private:
	bool m_owner = false;
	std::unique_ptr<T> m_handle;
};

#endif // __rhi_class_hpp__
