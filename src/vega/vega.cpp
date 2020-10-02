#include "etna/device.hpp"
#include "etna/image.hpp"
#include "etna/instance.hpp"
#include "etna/pipeline.hpp"
#include "etna/shader.hpp"

#include <spdlog/spdlog.h>

#include "C:/Program Files/RenderDoc/renderdoc_app.h"

int main()
{
#ifdef NDEBUG
    const bool enable_validation = false;
#else
    const bool enable_validation = true;
#endif

    LoadLibrary("C:\\Program Files\\RenderDoc\\renderdoc.dll");

    using namespace vk;

    RENDERDOC_API_1_4_0* rdoc_api = NULL;

    if (HMODULE mod = GetModuleHandleA("C:\\Program Files\\RenderDoc\\renderdoc.dll")) {
        pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
        int               ret              = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void**)&rdoc_api);
        assert(ret == 1);
    }

    std::vector<const char*> extensions;
    std::vector<const char*> layers;

    if (enable_validation) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        layers.push_back("VK_LAYER_KHRONOS_validation");
    }

    auto instance_owner  = etna::CreateUniqueInstance(extensions, layers);
    auto debug_owner     = etna::CreateUniqueDebugMessenger(instance_owner.get());
    auto device_owner    = etna::CreateUniqueDevice(instance_owner.get());
    auto allocator_owner = etna::CreateUniqueAllocator(device_owner.get());

    auto instance  = instance_owner.get();
    auto device    = device_owner.get();
    auto allocator = allocator_owner.get();

    if (rdoc_api)
        rdoc_api->StartFrameCapture(NULL, NULL);

    etna::UniqueImage2D image_owner;
    {
        auto format = Format::eR8G8B8A8Srgb;
        auto size   = Extent2D{ 256, 256 };
        auto usage  = ImageUsageFlagBits::eColorAttachment | ImageUsageFlagBits::eTransferSrc;
        auto memory = etna::MemoryUsage::eGpuOnly;

        image_owner = etna::CreateUniqueImage2D(allocator, format, size, usage, memory);
    }
    auto image = image_owner.get();

    UniqueRenderPass renderpass_owner;
    {
        etna::RenderPassBuilder builder;

        auto format         = image.Format();
        auto clear_on_load  = AttachmentLoadOp::eClear;
        auto write_on_store = AttachmentStoreOp::eStore;
        auto initial_layout = ImageLayout::eUndefined;
        auto final_layout   = ImageLayout::ePresentSrcKHR;

        auto attachment_id = builder.AddAttachment(format, clear_on_load, write_on_store, initial_layout, final_layout);
        auto reference_id  = builder.AddReference(attachment_id, ImageLayout::eColorAttachmentOptimal);

        builder.AddSubpass({ reference_id });

        renderpass_owner = device.createRenderPassUnique(builder.create_info);
    }
    auto renderpass = renderpass_owner.get();

    auto layout_owner = etna::CreateUniquePipelineLayout(device);
    auto layout       = layout_owner.get();

    UniquePipeline pipeline_owner;
    {
        auto builder         = etna::PipelineBuilder(layout, renderpass);
        auto vertex_shader   = etna::CreateUniqueShaderModule(device, "shader.vert");
        auto fragment_shader = etna::CreateUniqueShaderModule(device, "shader.frag");

        builder.AddShaderStage(vertex_shader.get(), ShaderStageFlagBits::eVertex);
        builder.AddShaderStage(fragment_shader.get(), ShaderStageFlagBits::eFragment);

        builder.AddViewport(image.Viewport());
        builder.AddScissor(image.Rect2D());

        builder.AddColorBlendAttachmentBaseState();

        pipeline_owner = device.createGraphicsPipelineUnique(nullptr, builder.create_info);
    }
    auto pipeline = pipeline_owner.get();

    auto image_view_owner = etna::CreateUniqueImageView(device, image);
    auto image_view       = image_view_owner.get();

    auto framebuffer_owner = etna::CreateUniqueFrameBuffer(device, renderpass, image.Extent(), { image_view });
    auto framebuffer       = framebuffer_owner.get();

    auto graphics_queue_family_index = etna::GetGraphicsQueueFamilyIndex(device);
    auto graphics_queue              = device.getQueue(graphics_queue_family_index, 0);

    auto command_pool_owner = device.createCommandPoolUnique({ {}, graphics_queue_family_index });
    auto command_pool       = command_pool_owner.get();

    auto command_buffer_owner = etna::AllocateUniqueCommandBuffer(device, command_pool);
    auto command_buffer       = command_buffer_owner.get();

    auto command_buffer_begin_info = vk::CommandBufferBeginInfo{};
    auto renderpass_begin_info     = GetRenderPassBeginInfo(renderpass, image.Rect2D(), framebuffer, image);

    command_buffer.begin(command_buffer_begin_info);
    command_buffer.beginRenderPass(renderpass_begin_info, SubpassContents::eInline);
    command_buffer.bindPipeline(PipelineBindPoint::eGraphics, pipeline);
    command_buffer.draw(3, 1, 0, 0);
    command_buffer.endRenderPass();
    command_buffer.end();

    SubmitInfo submit_info;
    {
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers    = &command_buffer;
    }

    auto fence = device.createFenceUnique({});

    graphics_queue.submit(submit_info, fence.get());

    device.waitForFences(fence.get(), true, UINT64_MAX);

    if (rdoc_api)
        rdoc_api->EndFrameCapture(NULL, NULL);

    return 0;
}
