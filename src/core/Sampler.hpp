
#ifndef __Sampler_h__
#define __Sampler_h__

#include "Common.hpp"

struct alignas(16) _SamplerConfig
{
    uint depth;
    float2 jitter;
    float padding; // 16 bytes alignment
};

class Sampler
{
};

#endif
