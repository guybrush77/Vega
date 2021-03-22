#include <texture_loader.hpp>

#include "utils/misc.hpp"

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

    auto promise = std::promise<StageBuffer>();

    promise.set_value(StageBuffer{ std::move(buffer), std::hash<std::string>{}("__default"), 1, 1 });

    m_tasks.push_back(promise.get_future());
}

void TextureLoader::LoadAsync(const std::string& filepath)
{
    m_tasks.push_back(std::async(std::launch::async, &TextureLoader::LoadAsyncPrivate, this, filepath));
}

void TextureLoader::UploadAsync()
{
    using namespace etna;

    if (m_tasks.empty()) {
        return;
    }

    m_command_buffer->ResetCommandBuffer(CommandBufferReset::ReleaseResources);

    m_command_buffer->Begin(CommandBufferUsage::OneTimeSubmit);

    for (auto& task : m_tasks) {
        task.wait();

        auto [buffer, hash, width, height] = task.get();

        auto image = m_device.CreateImage(
            Format::R8G8B8A8Srgb,
            { width, height },
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

        region.imageExtent = { width, height, 1 };

        m_command_buffer->CopyBufferToImage(*buffer, *image, ImageLayout::TransferDstOptimal, { region });

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

        m_gpu_images.insert({ hash, ImageRecord{ std::move(image), std::move(image_view) } });

        m_host_buffers.push_back(std::move(buffer));
    }

    m_command_buffer->End();

    m_transfer_queue.Submit(*m_command_buffer);

    m_tasks.clear();
}

void TextureLoader::CleanAfterUpload()
{
    m_host_buffers.clear();
}

etna::ImageView2D TextureLoader::GetImage(const std::string& image)
{
    auto hash = std::hash<std::string>{}(image);
    if (auto it = m_gpu_images.find(hash); it != m_gpu_images.end()) {
        return it->second.view.get();
    }
    return {};
}

etna::ImageView2D TextureLoader::GetDefaultImage()
{
    return GetImage("__default");
}

TextureLoader::StageBuffer TextureLoader::LoadAsyncPrivate(const std::string& filepath)
{
    using namespace etna;

    int  w, h, channels;
    auto pixels = static_cast<stbi_uc*>(stbi_load(filepath.c_str(), &w, &h, &channels, STBI_rgb_alpha));

    utils::throw_runtime_error_if(pixels == nullptr, stbi_failure_reason());

    auto width      = narrow_cast<uint32_t>(w);
    auto height     = narrow_cast<uint32_t>(h);
    auto image_size = STBI_rgb_alpha * width * height;

    auto buffer = m_device.CreateBuffer(image_size, BufferUsage::TransferSrc, MemoryUsage::CpuOnly);

    auto mapped_data = buffer->MapMemory();
    memcpy(mapped_data, pixels, image_size);
    buffer->UnmapMemory();

    stbi_image_free(pixels);

    return StageBuffer{ std::move(buffer), std::hash<std::string>{}(filepath), width, height };
}
