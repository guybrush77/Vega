#include <texture_loader.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

TextureLoader::TextureLoader(etna::Device device, etna::Queue transfer_queue)
    : m_device(device), m_transfer_queue(transfer_queue)
{
    using namespace etna;

    auto command_pool_flags = CommandPoolCreate::Transient | CommandPoolCreate::ResetCommandBuffer;

    m_command_pool   = m_device.CreateCommandPool(m_transfer_queue.FamilyIndex(), command_pool_flags);
    m_command_buffer = m_command_pool->AllocateCommandBuffer();

    stbi_set_flip_vertically_on_load(true);

    auto buffer = m_device.CreateBuffer(4, BufferUsage::TransferSrc, MemoryUsage::CpuOnly);

    uint8_t pixels[4] = { 0xFF, 0xFF, 0xFF, 0xFF };

    auto mapped_data = buffer->MapMemory();
    memcpy(mapped_data, pixels, 4);
    buffer->UnmapMemory();

    m_host_buffers["__default"] = StageBuffer{ std::move(buffer), 1, 1 };
}

void TextureLoader::LoadAsync(const std::string& base, const std::string& file)
{
    // TODO: make this async
    using namespace etna;

    if (m_host_buffers.count(file)) {
        return;
    }

    std::string path = base + "/" + file;

    int w, h, channels;

    auto pixels = static_cast<stbi_uc*>(stbi_load(path.c_str(), &w, &h, &channels, STBI_rgb_alpha));

    auto width      = narrow_cast<uint32_t>(w);
    auto height     = narrow_cast<uint32_t>(h);
    auto image_size = 4 * width * height;

    auto buffer = m_device.CreateBuffer(image_size, BufferUsage::TransferSrc, MemoryUsage::CpuOnly);

    auto mapped_data = buffer->MapMemory();
    memcpy(mapped_data, pixels, image_size);
    buffer->UnmapMemory();

    stbi_image_free(pixels);

    m_host_buffers[file] = StageBuffer{ std::move(buffer), width, height };
}

void TextureLoader::UploadAsync()
{
    using namespace etna;

    if (m_host_buffers.empty()) {
        return;
    }

    m_command_buffer->ResetCommandBuffer(CommandBufferReset::ReleaseResources);

    m_command_buffer->Begin(CommandBufferUsage::OneTimeSubmit);

    for (auto& [filename, buffer] : m_host_buffers) {
        auto image = m_device.CreateImage(
            Format::R8G8B8A8Srgb,
            { buffer.width, buffer.height },
            ImageUsage::TransferDst | ImageUsage::Sampled,
            MemoryUsage::GpuOnly,
            ImageTiling::Optimal);

        m_command_buffer->PipelineBarrier(
            *image,
            PipelineStage::TopOfPipe,
            PipelineStage::Transfer,
            {},
            Access::TransferWrite,
            ImageLayout::Undefined,
            ImageLayout::TransferDstOptimal,
            ImageAspect::Color);

        auto region = BufferImageCopy{};

        region.imageExtent = { buffer.width, buffer.height, 1 };

        m_command_buffer->CopyBufferToImage(buffer.buffer.get(), *image, ImageLayout::TransferDstOptimal, { region });

        m_command_buffer->PipelineBarrier(
            *image,
            PipelineStage::Transfer,
            PipelineStage::FragmentShader,
            Access::TransferWrite,
            Access::ShaderRead,
            ImageLayout::TransferDstOptimal,
            ImageLayout::ShaderReadOnlyOptimal,
            ImageAspect::Color);

        auto image_view = m_device.CreateImageView(*image, ImageAspect::Color);

        m_gpu_images.insert({ filename, ImageRecord{ std::move(image), std::move(image_view) } });
    }

    m_command_buffer->End();

    m_transfer_queue.Submit(*m_command_buffer);
}

void TextureLoader::CleanAfterUpload()
{
    m_host_buffers.clear();
}

etna::ImageView2D TextureLoader::GetImage(const std::string& image)
{
    if (auto it = m_gpu_images.find(image); it != m_gpu_images.end()) {
        return it->second.view.get();
    }
    return {};
}

etna::ImageView2D TextureLoader::GetDefaultImage()
{
    return GetImage("__default");
}
