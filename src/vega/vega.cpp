#include "etna/command.hpp"
#include "etna/device.hpp"
#include "etna/instance.hpp"
#include "etna/pipeline.hpp"
#include "etna/shader.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h" // TODO: Remove

#include <array>
#include <glm/vec3.hpp>
#include <vector>

DECLARE_VERTEX_ATTRIBUTE_TYPE(glm::vec3, etna::Format::R32G32B32Sfloat)

struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
};

std::array vertices = {

    Vertex{ { 0.0f, -0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f } },
    Vertex{ { 0.5f, 0.5f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
    Vertex{ { -0.5f, 0.5f, 1.0f }, { 0.0f, 0.0f, 1.0f } }
};

int main()
{
#ifdef NDEBUG
    const bool enable_validation = false;
#else
    const bool enable_validation = true;
#endif

    using namespace etna;

    std::vector<const char*> extensions;
    std::vector<const char*> layers;

    if (enable_validation) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        layers.push_back("VK_LAYER_KHRONOS_validation");
    }

    auto instance = CreateInstance("Vega", Version{ 0, 1, 0 }, extensions, layers);
    auto device   = instance->CreateDevice();

    // Copy vertices to GPU
    UniqueBuffer buffer;
    {
        auto src_size   = sizeof(vertices);
        auto src_usage  = BufferUsage::TransferSrc;
        auto src_memory = MemoryUsage::CpuOnly;
        auto src_buffer = device->CreateBuffer(src_size, src_usage, src_memory);

        void* data = src_buffer->MapMemory();
        memcpy(data, vertices.data(), src_size);
        src_buffer->UnmapMemory();

        auto dst_size   = src_size;
        auto dst_usage  = BufferUsage::VertexBuffer | BufferUsage::TransferDst;
        auto dst_memory = MemoryUsage::GpuOnly;
        buffer          = device->CreateBuffer(dst_size, dst_usage, dst_memory);

        auto cmd_pool   = device->CreateCommandPool(QueueFamily::Transfer, CommandPoolCreate::Transient);
        auto cmd_buffer = cmd_pool->AllocateCommandBuffer();

        cmd_buffer->Begin(CommandBufferUsage::OneTimeSubmit);
        cmd_buffer->CopyBuffer(*src_buffer, *buffer, src_size);
        cmd_buffer->End();

        device->GetQueue(QueueFamily::Transfer).Submit(*cmd_buffer);
        device->WaitIdle();
    }

    // Create render target image
    UniqueImage2D image;
    {
        auto format = Format::R8G8B8A8Srgb;
        auto usage  = ImageUsage::ColorAttachment | ImageUsage::TransferSrc;
        auto extent = Extent2D{ 256, 256 };
        auto memory = MemoryUsage::GpuOnly;
        auto tiling = ImageTiling::Optimal;
        image       = device->CreateImage(format, extent, usage, memory, tiling);
    }

    // Create pipeline renderpass
    UniqueRenderPass renderpass;
    {
        auto builder = RenderPassBuilder();

        auto format         = image->Format();
        auto clear_on_load  = AttachmentLoadOp::Clear;
        auto write_on_store = AttachmentStoreOp::Store;
        auto initial_layout = ImageLayout::Undefined;
        auto final_layout   = ImageLayout::TransferSrcOptimal;

        auto attachment_id = builder.AddAttachment(format, clear_on_load, write_on_store, initial_layout, final_layout);
        auto reference_id  = builder.AddReference(attachment_id, ImageLayout::ColorAttachmentOptimal);

        builder.AddSubpass(reference_id);

        renderpass = device->CreateRenderPass(builder.create_info);
    }

    // Create render target framebuffer
    auto image_view  = device->CreateImageView(*image);
    auto framebuffer = device->CreateFramebuffer(*renderpass, *image_view, image->Extent());

    // Create pipeline layout
    VkPipelineLayoutCreateInfo temp{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO }; // TODO: fixme
    auto                       layout = device->CreatePipelineLayout(temp);

    // Create pipeline
    UniquePipeline pipeline;
    {
        auto builder         = PipelineBuilder(*layout, *renderpass);
        auto vertex_shader   = device->CreateShaderModule("shader.vert");
        auto fragment_shader = device->CreateShaderModule("shader.frag");
        auto [width, height] = image->Extent();
        auto viewport        = Viewport{ 0, 0, static_cast<float>(width), static_cast<float>(height), 0, 1 };

        builder.AddShaderStage(vertex_shader.get(), ShaderStage::Vertex);
        builder.AddShaderStage(fragment_shader.get(), ShaderStage::Fragment);

        builder.AddVertexInputBindingDescription(Binding{ 0 }, sizeof(Vertex));

        builder.AddVertexInputAttributeDescription(
            Location{ 0 },
            Binding{ 0 },
            formatof(Vertex, position),
            offsetof(Vertex, position));

        builder.AddVertexInputAttributeDescription(
            Location{ 1 },
            Binding{ 0 },
            formatof(Vertex, color),
            offsetof(Vertex, color));

        builder.AddViewport(viewport);
        builder.AddScissor(image->Rect2D());

        builder.AddColorBlendAttachmentBaseState();

        pipeline = device->CreateGraphicsPipeline(builder.create_info);
    }

    // Render image
    {
        auto cmd_pool   = device->CreateCommandPool(QueueFamily::Graphics, CommandPoolCreate::Transient);
        auto cmd_buffer = cmd_pool->AllocateCommandBuffer();

        cmd_buffer->Begin(CommandBufferUsage::OneTimeSubmit);
        cmd_buffer->BeginRenderPass(*framebuffer, SubpassContents::Inline);
        cmd_buffer->BindPipeline(PipelineBindPoint::Graphics, *pipeline);
        cmd_buffer->BindVertexBuffers(*buffer);
        cmd_buffer->Draw(vertices.size(), 1, 0, 0);
        cmd_buffer->EndRenderPass();
        cmd_buffer->End();

        device->GetQueue(QueueFamily::Graphics).Submit(*cmd_buffer);
        device->WaitIdle();
    }

    // Transfer image to CPU
    UniqueImage2D dst_image;
    {
        auto format = image->Format();
        auto usage  = ImageUsage::TransferDst;
        auto extent = image->Extent();
        auto memory = MemoryUsage::CpuOnly;
        auto tiling = ImageTiling::Linear;
        dst_image   = device->CreateImage(format, extent, usage, memory, tiling);

        auto cmd_pool   = device->CreateCommandPool(QueueFamily::Transfer, CommandPoolCreate::Transient);
        auto cmd_buffer = cmd_pool->AllocateCommandBuffer();

        cmd_buffer->Begin();
        cmd_buffer->PipelineBarrier(
            *dst_image,
            PipelineStage::Transfer,
            PipelineStage::Transfer,
            Access{},
            Access::TransferWrite,
            ImageLayout::Undefined,
            ImageLayout::TransferDstOptimal,
            ImageAspect::Color);
        cmd_buffer->CopyImage(
            *image,
            ImageLayout::TransferSrcOptimal,
            *dst_image,
            ImageLayout::TransferDstOptimal,
            ImageAspect::Color);
        cmd_buffer->PipelineBarrier(
            *dst_image,
            PipelineStage::Transfer,
            PipelineStage::Transfer,
            Access::TransferWrite,
            Access::MemoryRead,
            ImageLayout::TransferDstOptimal,
            ImageLayout::General,
            ImageAspect::Color);
        cmd_buffer->End();

        device->GetQueue(QueueFamily::Transfer).Submit(*cmd_buffer);
        device->WaitIdle();
    }

    // Write image to file
    {
        auto width  = dst_image->Extent().width;
        auto height = dst_image->Extent().height;
        auto stride = 4 * width;

        void* data = dst_image->MapMemory();
        stbi_write_png("out.png", width, height, 4, data, stride);
        dst_image->UnmapMemory();
    }

    return 0;
}
