#include "vk_texture_2d.hpp"
#include "vk_buffers.hpp"

void get_texture_format_vk(VkFormat format, uint32_t& blockWidth, uint32_t& blockHeight, uint32_t& bytesPerBlock) {
    switch (format) {
    case VK_FORMAT_R8G8B8A8_UNORM:
    case VK_FORMAT_R8G8B8A8_SRGB:
    case VK_FORMAT_B8G8R8A8_UNORM:
        blockWidth = 1; blockHeight = 1; bytesPerBlock = 4;
        break;
    case VK_FORMAT_R16G16B16A16_SFLOAT:
        blockWidth = 1; blockHeight = 1; bytesPerBlock = 8;
        break;
    case VK_FORMAT_R32G32B32A32_SFLOAT:
        blockWidth = 1; blockHeight = 1; bytesPerBlock = 16;
        break;
        // Formatos Comprimidos (BC) -> Cada bloque es de 4x4 píxeles
    case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
    case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
        blockWidth = 4; blockHeight = 4; bytesPerBlock = 8;
        break;
    case VK_FORMAT_BC3_UNORM_BLOCK:
    case VK_FORMAT_BC3_SRGB_BLOCK:
    case VK_FORMAT_BC7_UNORM_BLOCK:
    case VK_FORMAT_BC7_SRGB_BLOCK:
        blockWidth = 4; blockHeight = 4; bytesPerBlock = 16;
        break;
    default:
        // Valor por defecto seguro para formatos estándar de 4 bytes
        blockWidth = 1; blockHeight = 1; bytesPerBlock = 4;
        break;
    }
}


RHI_TEXTURE_2D* vk_texture_2d_create(const RHI_TEXTURE_2D_DESC* const desc) {

	ASSERT_PTR(desc);
	ASSERT_PTR(desc->device);

	RHI_BUFFER_2D_DESC buff_desc = {};
	buff_desc.device = desc->device;
	buff_desc.format = desc->format;
	buff_desc.width = desc->width;
	buff_desc.height = desc->height;
	buff_desc.mips = desc->mips;
	buff_desc.type = buffer_type_image_2d;
	buff_desc.resource_flags = desc->resource_flags;
	VK_TEXTURE_2D* result = static_cast<VK_TEXTURE_2D*>(vk_buffers_create_2d(&buff_desc));
	ASSERT_PTR(result);

	std::vector<RHI_TEXTURE_MIPS> mips(desc->mips);
    uint32_t block_width, block_height, bytes_perBlock;
    get_texture_format_vk(vk_resource_format_type[desc->format], block_width, block_height, bytes_perBlock);

    size_t offset_accum = 0;
    uint32_t current_width = desc->width;
    uint32_t current_height = desc->height;
    uint32_t current_depth = 1;

    for (uint32_t i = 0; i < desc->mips; i++) {
        uint32_t num_blocks_x = (current_width + block_width - 1) / block_width;
        uint32_t num_blocks_y = (current_height + block_height - 1) / block_height;

        mips[i].width = current_width;
        mips[i].height = current_height;
        mips[i].depth = current_depth;
        mips[i].format = desc->format;
        mips[i].offset = offset_accum;

        mips[i].num_rows = num_blocks_y;

        mips[i].pitch = num_blocks_x * bytes_perBlock;

        size_t layer_size = static_cast<size_t>(mips[i].pitch) * num_blocks_y;
        size_t total_mip_size = layer_size * current_depth;

        offset_accum += total_mip_size;

        current_width = std::max(1u, current_width / 2);
        current_height = std::max(1u, current_height / 2);
        current_depth = std::max(1u, current_depth / 2);
    }

	VkMemoryRequirements mem_reqs;
	vkGetImageMemoryRequirements(*static_cast<VK_DEVICE*>(desc->device), *result, &mem_reqs);

	result->width = desc->width;
	result->height = desc->height;
	result->hw_length = static_cast<size_t>(mem_reqs.size);
	result->hw_format = result->format;
	result->mip_maps = std::move(mips);
	return result;

}
