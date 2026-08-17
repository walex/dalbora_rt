#ifndef LINEAR_ALGEBRA_HLSL
#define LINEAR_ALGEBRA_HLSL

inline float3x3 InverseTranspose3x3(float3x3 m)
{
    float3x3 cof;

    cof[0] = cross(m[1], m[2]);
    cof[1] = cross(m[2], m[0]);
    cof[2] = cross(m[0], m[1]);

    float det = dot(m[0], cof[0]);

    return cof / det;
}

#endif