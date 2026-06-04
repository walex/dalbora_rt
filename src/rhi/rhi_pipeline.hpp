#ifndef __rhi_pipeline_hpp__
#define __rhi_pipeline_hpp__

#include "rhi_impl.hpp"

template<typename T>
class RhiPipeline : public RhiImpl<T> {
public:
	RhiPipeline(T* ptr = nullptr) : RhiImpl<T>(ptr){}
	virtual ~RhiPipeline() = default;
};

#endif // __rhi_pipeline_hpp__