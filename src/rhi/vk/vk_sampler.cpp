#include "vk_sampler.hpp"

RHI_SAMPLER* vk_sampler_create(const RHI_SAMPLER_DESC* const desc) {

	VK_DEVICE* device_impl = static_cast<VK_DEVICE*>(desc->device);

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(device_impl->physical_device, &properties);

    // 2. Configurar la estructura de creación del sampler
    VkSamplerCreateInfo sampler_info{};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

    // Filtrado cuando la textura se ve más grande (mag) o más pequeña (min) que su resolución original
    sampler_info.magFilter = VK_FILTER_LINEAR; // VK_FILTER_NEAREST para estilo pixel art
    sampler_info.minFilter = VK_FILTER_LINEAR;

    // Comportamiento al salir de las coordenadas UV [0, 1]
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    // Filtrado anisotrópico (mejora la nitidez en ángulos inclinados)
    sampler_info.anisotropyEnable = VK_FALSE;
    sampler_info.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

    // Color del borde si se usa VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER
    sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

    // false = coordenadas normalizadas [0, 1). true = coordenadas en píxeles [0, ancho)
    sampler_info.unnormalizedCoordinates = VK_FALSE;

    // Comparación de profundidad (útil para Shadow Maps)
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;

    // Configuración de Mipmapping
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.mipLodBias = 0.0f;
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = VK_LOD_CLAMP_NONE; // Permite usar todos los niveles de mipmap disponibles

    // 3. Crear el objeto VkSampler
    VkSampler texture_sampler;
    if (vkCreateSampler(*device_impl, &sampler_info, nullptr, &texture_sampler) != VK_SUCCESS) {
        throw std::runtime_error("Error creating sampler texture.");
    }

    VK_SAMPLER* result = new VK_SAMPLER();
	result->parent_device = device_impl;
	result->set_handle(static_cast<RHI_VOID_PTR>(texture_sampler));
    ASSERT_PTR(result);
    return result;
}