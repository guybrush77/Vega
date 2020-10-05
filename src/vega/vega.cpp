#include "etna/command.hpp"
#include "etna/device.hpp"
//#include "etna/image.hpp"
#include "etna/instance.hpp"
#include "etna/pipeline.hpp"
#include "etna/shader.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h" // TODO: Remove

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

    auto instance_owner = CreateInstance("Vega", Version{ 0, 1, 0 }, extensions, layers);
    auto device_owner   = instance_owner->CreateDevice();
    auto instance       = instance_owner.get();
    auto device         = device_owner.get();

    auto image_owner = device.CreateImage(
        Format::R8G8B8A8Srgb,
        Extent2D{ 256, 256 },
        ImageUsage::ColorAttachment | ImageUsage::TransferSrc,
        MemoryUsage::GpuOnly,
        ImageTiling::Optimal);
    auto image = image_owner.get();

    auto image_view_owner = device.CreateImageView(image);
    auto image_view       = image_view_owner.get();

    UniqueRenderPass renderpass_owner;
    {
        auto builder = RenderPassBuilder();

        auto format         = image.Format();
        auto clear_on_load  = AttachmentLoadOp::Clear;
        auto write_on_store = AttachmentStoreOp::Store;
        auto initial_layout = ImageLayout::Undefined;
        auto final_layout   = ImageLayout::TransferSrcOptimal;

        auto attachment_id = builder.AddAttachment(format, clear_on_load, write_on_store, initial_layout, final_layout);
        auto reference_id  = builder.AddReference(attachment_id, ImageLayout::ColorAttachmentOptimal);

        builder.AddSubpass(reference_id);

        renderpass_owner = device.CreateRenderPass(builder.create_info);
    }
    auto renderpass = renderpass_owner.get();

    VkPipelineLayoutCreateInfo temp{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO }; // TODO: fixme

    auto layout_owner = device.CreatePipelineLayout(temp);
    auto layout       = layout_owner.get();

    UniquePipeline pipeline_owner;
    {
        auto builder         = PipelineBuilder(layout, renderpass);
        auto vertex_shader   = device.CreateShaderModule("shader.vert");
        auto fragment_shader = device.CreateShaderModule("shader.frag");
        auto [width, height] = image.Extent();
        auto viewport        = Viewport{ 0, 0, static_cast<float>(width), static_cast<float>(height), 0, 1 };

        builder.AddShaderStage(vertex_shader.get(), ShaderStage::Vertex);
        builder.AddShaderStage(fragment_shader.get(), ShaderStage::Fragment);

        builder.AddViewport(viewport);
        builder.AddScissor(image.Rect2D());

        builder.AddColorBlendAttachmentBaseState();

        pipeline_owner = device.CreateGraphicsPipeline(builder.create_info);
    }
    auto pipeline = pipeline_owner.get();

    auto framebuffer_owner = device.CreateFramebuffer(renderpass, image_view, image.Extent());
    auto framebuffer       = framebuffer_owner.get();

    auto graphics_queue = device.GetQueue(QueueFamily::Graphics);

    auto command_pool_owner = device.CreateCommandPool(QueueFamily::Graphics);
    auto command_pool       = command_pool_owner.get();

    // Render image
    {
        auto cmd_buffer_owner = command_pool.AllocateCommandBuffer();
        auto cmd_buffer       = cmd_buffer_owner.get();

        cmd_buffer.Begin();
        cmd_buffer.BeginRenderPass(framebuffer, SubpassContents::Inline);
        cmd_buffer.BindPipeline(PipelineBindPoint::Graphics, pipeline);
        cmd_buffer.Draw(3, 1, 0, 0);
        cmd_buffer.EndRenderPass();
        cmd_buffer.End();

        graphics_queue.Submit(cmd_buffer);

        device.WaitIdle();
    }

    auto dst_image_owner = device.CreateImage(
        image.Format(),
        image.Extent(),
        ImageUsage::TransferDst,
        MemoryUsage::CpuOnly,
        ImageTiling::Linear);
    auto dst_image = dst_image_owner.get();

    // Transfer image to CPU
    {
        auto cmd_buffer_owner = command_pool.AllocateCommandBuffer();
        auto cmd_buffer       = cmd_buffer_owner.get();

        cmd_buffer.Begin();
        cmd_buffer.PipelineBarrier(
            dst_image,
            PipelineStage::Transfer,
            PipelineStage::Transfer,
            Access{},
            Access::TransferWrite,
            ImageLayout::Undefined,
            ImageLayout::TransferDstOptimal,
            ImageAspect::Color);
        cmd_buffer.CopyImage(
            image,
            ImageLayout::TransferSrcOptimal,
            dst_image,
            ImageLayout::TransferDstOptimal,
            ImageAspect::Color);
        cmd_buffer.PipelineBarrier(
            dst_image,
            PipelineStage::Transfer,
            PipelineStage::Transfer,
            Access::TransferWrite,
            Access::MemoryRead,
            ImageLayout::TransferDstOptimal,
            ImageLayout::General,
            ImageAspect::Color);
        cmd_buffer.End();

        graphics_queue.Submit(cmd_buffer);

        device.WaitIdle();
    }

    void* data = dst_image.MapMemory();

    auto width  = dst_image.Extent().width;
    auto height = dst_image.Extent().height;
    auto stride = 4 * width;

    stbi_write_png("out.png", width, height, 4, data, stride);

    dst_image.UnmapMemory();

    return 0;
}
