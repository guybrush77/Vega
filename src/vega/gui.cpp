#include "gui.hpp"

#include "command.hpp"
#include "synchronization.hpp"

#include "utils/resource.hpp"

#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_vulkan.h"
#include "imgui.h"

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#endif

#include <spdlog/spdlog.h>

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

Gui::Gui(
    etna::Instance       instance,
    etna::PhysicalDevice gpu,
    etna::Device         device,
    uint32_t             queue_family_index,
    etna::Queue          graphics_queue,
    etna::RenderPass     renderpass,
    GLFWwindow*          window,
    etna::Extent2D       extent,
    uint32_t             min_image_count,
    uint32_t             image_count)
    : m_graphics_queue(graphics_queue), m_extent(extent)
{
    using namespace etna;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    spdlog::info("ImGui {}", ImGui::GetVersion());

    // Create Descriptor Pool
    {
        DescriptorPoolSize pool_sizes[] = {

            { DescriptorType::Sampler, 1000 },
            { DescriptorType::CombinedImageSampler, 1000 },
            { DescriptorType::SampledImage, 1000 },
            { DescriptorType::StorageImage, 1000 },
            { DescriptorType::UniformTexelBuffer, 1000 },
            { DescriptorType::StorageTexelBuffer, 1000 },
            { DescriptorType::UniformBuffer, 1000 },
            { DescriptorType::StorageBuffer, 1000 },
            { DescriptorType::UniformBufferDynamic, 1000 },
            { DescriptorType::StorageBufferDynamic, 1000 },
            { DescriptorType::InputAttachment, 1000 }
        };
        m_descriptor_pool = device.CreateDescriptorPool(pool_sizes);
    }

    // Init glfw
    ImGui_ImplGlfw_InitForVulkan(window, true);

    // Init Vulkan
    {
        auto init_info = ImGui_ImplVulkan_InitInfo{

            .Instance        = instance,
            .PhysicalDevice  = gpu,
            .Device          = device,
            .QueueFamily     = queue_family_index,
            .Queue           = graphics_queue,
            .PipelineCache   = nullptr,
            .DescriptorPool  = *m_descriptor_pool,
            .MinImageCount   = min_image_count,
            .ImageCount      = image_count,
            .MSAASamples     = VK_SAMPLE_COUNT_1_BIT,
            .Allocator       = nullptr,
            .CheckVkResultFn = nullptr
        };
        ImGui_ImplVulkan_Init(&init_info, renderpass);
    }

    // Set Style
    ImGui::StyleColorsDark();

    // Add fonts
    {
        auto font      = GetResource("fonts/Roboto-Regular.ttf");
        auto font_data = const_cast<unsigned char*>(font.data);
        auto font_size = narrow_cast<int>(font.size);

        ImFontConfig font_config;
        font_config.FontDataOwnedByAtlas = false;

        ImGui::GetIO().Fonts->AddFontFromMemoryTTF(font_data, font_size, 24.0f, &font_config);
    }

    // Upload Fonts
    {
        auto command_pool   = device.CreateCommandPool(queue_family_index, CommandPoolCreate::Transient);
        auto command_buffer = command_pool->AllocateCommandBuffer();

        command_buffer->Begin(CommandBufferUsage::OneTimeSubmit);
        ImGui_ImplVulkan_CreateFontsTexture(*command_buffer);
        command_buffer->End();

        graphics_queue.Submit(*command_buffer);

        device.WaitIdle();
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }
}

Gui::~Gui() noexcept
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Gui::Update(etna::Extent2D extent, uint32_t min_image_count)
{
    ImGui_ImplVulkan_SetMinImageCount(min_image_count);
    m_extent = extent;
}

void Gui::Draw(
    etna::CommandBuffer cmd_buffer,
    etna::Framebuffer   framebuffer,
    etna::Semaphore     wait_semaphore,
    etna::Semaphore     signal_semaphore,
    etna::Fence         finished_fence)
{
    using namespace etna;

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Hello, world!");
    ImGui::Text("This is some useful text.");
    ImGui::End();

    ImGui::Render();

    auto  render_area = Rect2D{ Offset2D{ 0, 0 }, m_extent };
    auto  clear_color = ClearColor::Transparent;
    auto* draw_data   = ImGui::GetDrawData();

    cmd_buffer.ResetCommandBuffer();
    cmd_buffer.Begin();
    cmd_buffer.BeginRenderPass(framebuffer, render_area, { clear_color });
    ImGui_ImplVulkan_RenderDrawData(draw_data, cmd_buffer);
    cmd_buffer.EndRenderPass();
    cmd_buffer.End();

    m_graphics_queue.Submit(
        cmd_buffer,
        { wait_semaphore },
        { PipelineStage::ColorAttachmentOutput },
        { signal_semaphore },
        finished_fence);
}
