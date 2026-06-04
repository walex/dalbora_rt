#ifndef __rhi_shader_binding_table_hpp__
#define __rhi_shader_binding_table_hpp__

#include "rhi_impl.hpp"

class RhiDevice;
class RhiRayTracePipeline;
class RhiRayTracePipelineShaderPrograms;
class RhiShaderBindingTable
    : public RhiImpl<RHI_SBT_TABLE>
    , public ICreateRhiObject<const RhiDevice&, const RhiRayTracePipeline&, 
                                const RhiRayTracePipelineShaderPrograms&> {

public:
    RhiShaderBindingTable(RHI_SBT_TABLE* handle = nullptr);
    virtual ~RhiShaderBindingTable() = default;
    void create(const RhiDevice& device, const RhiRayTracePipeline& pipeline,
        const RhiRayTracePipelineShaderPrograms& shader_programs);

};
#endif // __rhi_sbt_hpp__