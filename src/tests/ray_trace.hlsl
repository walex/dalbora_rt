// =========================================================================
// 1. ESTRUCTURAS Y CONSTANT BUFFERS
// =========================================================================

struct RayPayload
{
    float3 ColorAccumulated; // Luz acumulada en el camino
    float3 Throughput;       // Energía restante (Multiplicador de Albedo)
    float3 NextOrigin;       // Origen del siguiente rebote
    float3 NextDirection;    // Dirección del siguiente rebote
    uint Seed;               // Semilla aleatoria única para este hilo/muestra
    bool IsTerminated;       // Bandera para romper el bucle iterativo
};

cbuffer CameraBuffer : register(b0)
{
    float4x4 InvViewProjection; // Matriz para transformar de pantalla a espacio 3D
    uint Width;                 // Resolución X (ej: 3840)
    uint Height;                // Resolución Y (ej: 2160)
    uint SamplesPerPixel;       // Cantidad total de muestras dinámicas
    uint FrameCount;            // Incrementado en la CPU cada fotograma
};

// Recursos nativos de DirectX Raytracing (DXR)
RaytracingAccelerationStructure SceneBVH : register(t0); // Estructura TLAS
RWTexture2D<float4> RenderTarget        : register(u0); // Buffer de pantalla flotante

// =========================================================================
// 2. FUNCIONES MATEMÁTICAS (Halton, Hash y Muestreo)
// =========================================================================

// Generador de Halton (Inversión Radical) ejecutado localmente "al vuelo"
float Halton(uint index, uint base)
{
    float result = 0.0f;
    float f = 1.0f / (float)base;
    uint i = index;
    while (i > 0)
    {
        result += f * (float)(i % base);
        i = i / base;
        f = f / base;
    }
    return result;
}

// Generador de números aleatorios rápido y de alta calidad para GPU (PCG Hash)
uint InitPCG(uint2 id, uint frame)
{
    return id.x * 1664525u + id.y * 1013904223u + frame * 392911u;
}

float NextRandomFloat(inout uint seed)
{
    seed = seed * 747796405u + 289133645u;
    uint word = ((seed >> ((seed >> 28u) + 4u)) ^ seed) * 277803737u;
    uint result = (word >> 22u) ^ word;
    return (float)result / 4294967296.0f;
}

// Crear base ortonormal para orientar el hemisferio respecto a la normal
void GetOrthonormalBasis(float3 normal, out float3 tangent, out float3 bitangent)
{
    float3 up = abs(normal.z) < 0.999f ? float3(0.0f, 0.0f, 1.0f) : float3(1.0f, 0.0f, 0.0f);
    tangent = normalize(cross(up, normal));
    bitangent = cross(normal, tangent);
}

// Muestreo con importancia coseno (Cosine-Weighted Hemispherical Sampling)
float3 GenerateRandomCosineDirection(float3 hitNormal, inout uint seed)
{
    float r1 = NextRandomFloat(seed);
    float r2 = NextRandomFloat(seed);

    float phi = 2.0f * 3.14159265f * r1;
    float cosTheta = sqrt(r2);
    float sinTheta = sqrt(max(0.0f, 1.0f - r2));

    float3 localDir;
    localDir.x = cos(phi) * sinTheta;
    localDir.y = sin(phi) * sinTheta;
    localDir.z = cosTheta;

    float3 tangent, bitangent;
    GetOrthonormalBasis(hitNormal, tangent, bitangent);

    return normalize(tangent * localDir.x + bitangent * localDir.y + hitNormal * localDir.z);
}

// =========================================================================
// 3. SHADERS DEL PIPELINE DE TRAZADO DE RAYOS
// =========================================================================

[shader("RayGen")]
void MyRayGenShader()
{
    uint3 dispatchIndex = DispatchRaysIndex();
    uint2 pixelCoords = dispatchIndex.xy; 
    uint sampleIndex = dispatchIndex.z; // Controlado por el parámetro Depth en C++

    // Asegurar que no procesamos fuera de la pantalla (si el dispatch es mayor)
    if (pixelCoords.x >= Width || pixelCoords.y >= Height) return;

    // Inicializar semilla aleatoria única combinando coordenadas y el índice del sample
    uint seed = InitPCG(pixelCoords, FrameCount + sampleIndex);

    // Calcular Halton "al vuelo" aplicando el desplazamiento Cranley-Patterson
    float pixelShiftX = NextRandomFloat(seed);
    float pixelShiftY = NextRandomFloat(seed);

    float hX = Halton(sampleIndex + 1, 2);
    float hY = Halton(sampleIndex + 1, 3);

    float sampleOffsetX = frac(hX + pixelShiftX);
    float sampleOffsetY = frac(hY + pixelShiftY);

    // Mapear a coordenadas UV globales de pantalla (0.0 a 1.0)
    float2 uv = (float2(pixelCoords) + float2(sampleOffsetX, sampleOffsetY)) / float2(Width, Height);

    // Construcción matemática del rayo primario (Espacio NDC a Mundo)
    float2 ndc = uv * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f);
    float4 target = mul(InvViewProjection, float4(ndc, 0.0f, 1.0f)); target /= target.w;
    float4 origin = mul(InvViewProjection, float4(ndc.xy, 1.0f, 1.0f)); origin /= origin.w;

    RayDesc ray;
    ray.Origin = origin.xyz;
    ray.Direction = normalize(target.xyz - origin.xyz);
    ray.TMin = 0.001f;
    ray.TMax = 1000.0f;

    // Inicializar los datos físicos del camino lumínico (Path)
    RayPayload payload;
    payload.ColorAccumulated = float3(0.0f, 0.0f, 0.0f);
    payload.Throughput       = float3(1.0f, 1.0f, 1.0f);
    payload.Seed             = seed;
    payload.IsTerminated     = false;

    // Bucle iterativo de rebotes de luz (Física de Path Tracing)
    const uint MAX_BOUNCES = 4;
    for (uint bounce = 0; bounce < MAX_BOUNCES; ++bounce)
    {
        // Ejecuta síncronamente Closest Hit o Miss
        TraceRay(SceneBVH, RAY_FLAG_NONE, 0xFF, 0, 1, 0, ray, payload);

        if (payload.IsTerminated) break;

        // El RayGen actualiza el origen y dirección con lo que el Closest Hit calculó
        ray.Origin    = payload.NextOrigin;
        ray.Direction = payload.NextDirection;
    }

    // Promediar la muestra de forma individual
    float3 sampleContribution = payload.ColorAccumulated / (float)SamplesPerPixel;

    // Shader Model 6.6: Suma atómica flotante directa en el RenderTarget compartido
    InterlockedAdd(RenderTarget[pixelCoords].r, sampleContribution.r);
    InterlockedAdd(RenderTarget[pixelCoords].g, sampleContribution.g);
    InterlockedAdd(RenderTarget[pixelCoords].b, sampleContribution.b);
}

[shader("Miss")]
void MyMissShader(inout RayPayload payload)
{
    // Color de cielo degradado básico
    float3 rayDir = WorldRayDirection();
    float t = 0.5f * (rayDir.y + 1.0f);
    float3 skyColor = lerp(float3(1.0f, 1.0f, 1.0f), float3(0.5f, 0.7f, 1.0f), t);

    // Sumar la luz ambiental multiplicada por la energía restante
    payload.ColorAccumulated += payload.Throughput * skyColor;
    payload.IsTerminated = true;
}

[shader("ClosestHit")]
void MyClosestHitShader(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    // Calcular punto de impacto
    float3 hitPosition = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
    
    // Normal aproximada (En un motor real leerías el Vertex Buffer usando PrimitiveIndex())
    float3 hitNormal = float3(0.0f, 1.0f, 0.0f); 

    // Propiedades del material (Ejemplo: Pared Roja Difusa)
    float3 albedo = float3(0.8f, 0.1f, 0.1f);
    float3 emissive = float3(0.0f, 0.0f, 0.0f);

    // Acumular auto-emisión del objeto
    payload.ColorAccumulated += payload.Throughput * emissive;

    // Absorción de la luz según el color del material
    payload.Throughput *= albedo;

    // Ruleta Rusa simplificada para detener rayos sin energía
    if (max(payload.Throughput.r, max(payload.Throughput.g, payload.Throughput.b)) < 0.01f)
    {
        payload.IsTerminated = true;
        return;
    }

    // Calcular datos del siguiente rebote y guardarlos en el payload
    payload.NextOrigin = hitPosition + hitNormal * 0.001f; // Offset contra píxel acné
    payload.NextDirection = GenerateRandomCosineDirection(hitNormal, payload.Seed);
}

// dispatchDesc.Depth  = miCantidadSamples; 