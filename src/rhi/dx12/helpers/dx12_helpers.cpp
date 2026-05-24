#include "dx12_helpers.hpp"
#include "dx12_rhi.hpp"

resource_format dx12_helpers_resource_format_from_dxgi_format(const DXGI_FORMAT format) {

    std::span<const DXGI_FORMAT> s(dx12_resource_format_type);
    auto it = std::find(s.begin(), s.end(), format);
    if (it == s.end())
    {
        throw std::exception("texture format no supported");
    }
    return static_cast<resource_format>(std::distance(s.begin(), it));
}