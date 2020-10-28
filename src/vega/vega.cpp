#include "etna.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "utils/resource.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtx/transform.hpp>
#include <glm/matrix.hpp>

#include <algorithm>
#include <optional>
#include <spdlog/spdlog.h>
#include <unordered_map>
#include <vector>

struct GLFW {
    GLFW() { glfwInit(); }
    ~GLFW() { glfwTerminate(); }
} glfw;

DECLARE_VERTEX_ATTRIBUTE_TYPE(glm::vec3, etna::Format::R32G32B32Sfloat)

struct MVP {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

struct VertexPN {
    glm::vec3 position;
    glm::vec3 normal;
};

struct Mesh final {
    std::string name;

    struct {
        std::vector<VertexPN> array;

        auto count() const noexcept { return static_cast<uint32_t>(array.size()); }
        auto size() const noexcept { return static_cast<uint32_t>(sizeof(array[0]) * array.size()); }
        auto data() const noexcept { return array.data(); }
    } vertices;

    struct {
        std::vector<uint32_t> array;

        auto count() const noexcept { return static_cast<uint32_t>(array.size()); }
        auto size() const noexcept { return static_cast<uint32_t>(sizeof(array[0]) * array.size()); }
        auto data() const noexcept { return array.data(); }
    } indices;
};

struct Index final {
    Index() noexcept = default;

    constexpr Index(tinyobj::index_t idx) noexcept
        : vertex(idx.vertex_index), normal(idx.normal_index), texcoord(idx.texcoord_index)
    {}

    constexpr size_t operator()(const Index& index) const noexcept
    {
        size_t hash = 23;
        hash        = hash * 31 + index.vertex;
        hash        = hash * 31 + index.normal;
        hash        = hash * 31 + index.texcoord;
        return hash;
    }

    int vertex;
    int normal;
    int texcoord;
};

constexpr bool operator==(const Index& lhs, const Index& rhs) noexcept
{
    return lhs.vertex == rhs.vertex && lhs.normal == rhs.normal && lhs.texcoord == rhs.texcoord;
}

Mesh LoadMesh(const char* filepath)
{
    tinyobj::attrib_t                attributes;
    std::vector<tinyobj::shape_t>    shapes;
    std::vector<tinyobj::material_t> materials;
    std::string                      warning;
    std::string                      err;
    bool success = tinyobj::LoadObj(&attributes, &shapes, &materials, &warning, &err, filepath, nullptr, true, false);

    auto index_map = std::unordered_map<Index, int, Index>{};
    auto vertices  = std::vector<VertexPN>{};
    auto indices   = std::vector<uint32_t>{};

    auto shape       = shapes[0];
    auto num_indices = shape.mesh.indices.size();

    assert(num_indices % 3 == 0);

    for (size_t i = 0; i < num_indices; i += 3) {
        for (std::size_t j = 0; j < 3; ++j) {
            auto index = Index(shape.mesh.indices[i + j]);
            if (auto it = index_map.find(index); it != index_map.end()) {
                indices.push_back(it->second);
            } else {
                VertexPN vertex;
                {
                    auto position_idx = static_cast<size_t>(3) * index.vertex;

                    vertex.position.x = attributes.vertices[position_idx + 0];
                    vertex.position.y = attributes.vertices[position_idx + 1];
                    vertex.position.z = attributes.vertices[position_idx + 2];

                    auto normal_idx = static_cast<size_t>(3) * index.normal;

                    vertex.normal.x = attributes.normals[normal_idx + 0];
                    vertex.normal.y = attributes.normals[normal_idx + 1];
                    vertex.normal.z = attributes.normals[normal_idx + 2];
                }

                vertices.push_back(vertex);

                auto idx         = static_cast<int>(vertices.size() - 1);
                index_map[index] = idx;

                indices.push_back(idx);
            }
        }
    }

    return { std::move(shape.name), { std::move(vertices) }, { std::move(indices) } };
}

struct QueueInfo final {
    uint32_t         family_index;
    etna::QueueFlags flags;
    uint32_t         count;
};

struct QueueFamilies final {
    QueueInfo graphics;
    QueueInfo compute;
    QueueInfo transfer;
    QueueInfo presentation;
};

struct Queues final {
    etna::Queue graphics;
    etna::Queue compute;
    etna::Queue transfer;
    etna::Queue presentation;
};

template <typename T>
std::vector<T> RemoveDuplicates(std::initializer_list<T> l)
{
    std::vector<T> v(l.begin(), l.end());
    std::sort(v.begin(), v.end());
    v.erase(std::unique(v.begin(), v.end()), v.end());
    return v;
}

QueueFamilies GetQueueFamilyInfo(etna::PhysicalDevice gpu, etna::SurfaceKHR surface)
{
    using namespace etna;

    auto properties = gpu.GetPhysicalDeviceQueueFamilyProperties();

    constexpr auto mask = QueueFlags::Graphics | QueueFlags::Compute | QueueFlags::Transfer;

    std::optional<QueueInfo> graphics;

    std::optional<QueueInfo> presentation;
    std::optional<QueueInfo> graphics_presentation;
    std::optional<QueueInfo> mixed_presentation;

    std::optional<QueueInfo> compute;
    std::optional<QueueInfo> dedicated_compute;
    std::optional<QueueInfo> graphics_compute;
    std::optional<QueueInfo> mixed_compute;

    std::optional<QueueInfo> transfer;
    std::optional<QueueInfo> dedicated_transfer;
    std::optional<QueueInfo> graphics_transfer;
    std::optional<QueueInfo> mixed_transfer;

    for (std::size_t i = 0; i != properties.size(); ++i) {
        const auto family_index = narrow_cast<uint32_t>(i);
        const auto queue_flags  = properties[i].queueFlags;
        const auto queue_count  = properties[i].queueCount;
        const auto masked_flags = queue_flags & mask;
        const auto queue_info   = QueueInfo{ family_index, queue_flags, queue_count };

        if (masked_flags & QueueFlags::Graphics) {
            if (!graphics.has_value() || queue_count > graphics->count) {
                graphics = queue_info;
            }
        }

        if (masked_flags == QueueFlags::Compute) {
            if (!dedicated_compute.has_value() || queue_count > dedicated_compute->count) {
                dedicated_compute = queue_info;
            }
        } else if (masked_flags & QueueFlags::Compute) {
            if (masked_flags & QueueFlags::Graphics) {
                if (!graphics_compute.has_value() || queue_count > graphics_compute->count) {
                    graphics_compute = queue_info;
                }
            } else {
                if (!mixed_compute.has_value() || queue_count > mixed_compute->count) {
                    mixed_compute = queue_info;
                }
            }
        }

        if (masked_flags == QueueFlags::Transfer) {
            if (!dedicated_transfer.has_value() || queue_count > dedicated_transfer->count) {
                dedicated_transfer = queue_info;
            }
        } else if (masked_flags & QueueFlags::Transfer) {
            if (masked_flags & QueueFlags::Graphics) {
                if (!graphics_transfer.has_value() || queue_count > graphics_transfer->count) {
                    graphics_transfer = queue_info;
                }
            } else {
                if (!mixed_transfer.has_value() || queue_count > mixed_transfer->count) {
                    mixed_transfer = queue_info;
                }
            }
        }

        if (gpu.GetPhysicalDeviceSurfaceSupportKHR(family_index, surface)) {
            if (masked_flags & QueueFlags::Graphics) {
                if (!graphics_presentation.has_value()) {
                    graphics_presentation = queue_info;
                }
            } else {
                mixed_presentation = queue_info;
            }
        }
    }

    if (false == graphics.has_value()) {
        throw_runtime_error("Failed to detect GPU graphics queue!");
    }

    if (dedicated_compute.has_value()) {
        compute = dedicated_compute;
    } else if (mixed_compute.has_value()) {
        compute = mixed_compute;
    } else if (graphics_compute.has_value()) {
        compute = graphics_compute;
    }

    if (false == compute.has_value()) {
        throw_runtime_error("Failed to detect GPU compute queue!");
    }

    if (dedicated_transfer.has_value()) {
        transfer = dedicated_transfer;
    } else if (mixed_transfer.has_value()) {
        transfer = mixed_transfer;
    } else if (graphics_transfer.has_value()) {
        transfer = graphics_transfer;
    }

    if (false == transfer.has_value()) {
        throw_runtime_error("Failed to detect GPU transfer queue!");
    }

    if (graphics_presentation.has_value()) {
        presentation = graphics_presentation;
    } else if (mixed_presentation.has_value()) {
        presentation = mixed_presentation;
    }

    if (false == presentation.has_value()) {
        throw_runtime_error("Failed to detect GPU presentation queue!");
    }

    return { graphics.value(), compute.value(), transfer.value(), presentation.value() };
}

etna::SurfaceFormatKHR FindOptimalSurfaceFormatKHR(
    etna::PhysicalDevice                gpu,
    etna::SurfaceKHR                    surface,
    etna::ArrayView<etna::SurfaceFormatKHR> preffered_formats)
{
    auto available_formats = gpu.GetPhysicalDeviceSurfaceFormatsKHR(surface);

    if (available_formats.empty()) {
        throw std::runtime_error("Failed to find supported surface format!");
    }

    for (auto preffered_format : preffered_formats) {
        if (std::ranges::count(available_formats, preffered_format)) {
            return preffered_format;
        }
    }

    return available_formats.front();
}

etna::Format FindSupportedFormat(
    etna::PhysicalDevice      gpu,
    etna::ArrayView<etna::Format> candidate_formats,
    etna::ImageTiling         tiling,
    etna::FormatFeature       required_features)
{
    using namespace etna;

    for (auto format : candidate_formats) {
        auto format_features = gpu.GetPhysicalDeviceFormatProperties(format);
        if (tiling == ImageTiling::Linear) {
            if (required_features == (format_features.linearTilingFeatures & required_features)) {
                return format;
            }
        } else if (tiling == ImageTiling::Optimal) {
            if (required_features == (format_features.optimalTilingFeatures & required_features)) {
                return format;
            }
        }
    }

    throw std::runtime_error("Failed to find supported depth format!");

    return Format::Undefined;
}

struct Frame {
    etna::CommandBuffer command_buffer;
    etna::Semaphore     image_acquired_semaphore;
    etna::Semaphore     image_rendered_semaphore;
    etna::Fence         frame_available_fence;
    uint32_t            index;
};

class FrameManager {
  public:
    FrameManager(etna::Device device, uint32_t queue_family_index, uint32_t max_frames)
        : m_device(device), m_max_frames(max_frames), m_current_frame(0)
    {
        m_command_pool = device.CreateCommandPool(queue_family_index, etna::CommandPoolCreate::ResetCommandBuffer);

        for (uint32_t frame_index = 0; frame_index < max_frames; ++frame_index) {
            m_command_buffers.push_back(m_command_pool->AllocateCommandBuffer());
            m_image_acquired_sempahores.push_back(device.CreateSemaphore());
            m_image_rendered_sempahores.push_back(device.CreateSemaphore());
            m_frame_available_fences.push_back(device.CreateFence(etna::FenceCreate::Signaled));
            m_frames.push_back(Frame{ *m_command_buffers.back(),
                                      *m_image_acquired_sempahores.back(),
                                      *m_image_rendered_sempahores.back(),
                                      *m_frame_available_fences.back(),
                                      frame_index });
        }
    }

    Frame NextFrame()
    {
        auto index      = m_current_frame;
        m_current_frame = (m_current_frame + 1) % m_max_frames;
        auto fence      = m_frames[index].frame_available_fence;

        m_device.WaitForFence(fence);
        m_device.ResetFence(fence);

        return m_frames[index];
    }

  private:
    etna::Device                           m_device;
    etna::UniqueCommandPool                m_command_pool;
    std::vector<etna::UniqueCommandBuffer> m_command_buffers;
    std::vector<etna::UniqueSemaphore>     m_image_acquired_sempahores;
    std::vector<etna::UniqueSemaphore>     m_image_rendered_sempahores;
    std::vector<etna::UniqueFence>         m_frame_available_fences;
    std::vector<Frame>                     m_frames;
    uint32_t                               m_max_frames;
    uint32_t                               m_current_frame;
};

class DescriptorManager {
  public:
    DescriptorManager(etna::Device device, size_t count, etna::DescriptorSetLayout descriptor_set_layout)
        : m_device(device)
    {
        using namespace etna;

        m_descriptor_pool = device.CreateDescriptorPool(DescriptorType::UniformBuffer, count);
        m_descriptor_sets = m_descriptor_pool->AllocateDescriptorSets(count, descriptor_set_layout);
        m_descriptor_buffers =
            device.CreateBuffers(count, sizeof(MVP), BufferUsage::UniformBuffer, MemoryUsage::CpuToGpu);
    }

    auto DescriptorSet(size_t index) const noexcept { return m_descriptor_sets[index]; }

    void UpdateDescriptorSet(size_t index, const MVP& mvp)
    {
        using namespace etna;

        void* data = m_descriptor_buffers[index]->MapMemory();
        memcpy(data, &mvp, sizeof(mvp));
        m_descriptor_buffers[index]->UnmapMemory();

        WriteDescriptorSet write_descriptor(m_descriptor_sets[index], Binding{ 0 }, DescriptorType::UniformBuffer);
        write_descriptor.AddBuffer(*m_descriptor_buffers[index]);
        m_device.UpdateDescriptorSet(write_descriptor);
    }

  private:
    etna::Device                     m_device;
    etna::UniqueDescriptorPool       m_descriptor_pool;
    std::vector<etna::DescriptorSet> m_descriptor_sets;
    std::vector<etna::UniqueBuffer>  m_descriptor_buffers;
};

class SwapchainManager {
  public:
    SwapchainManager(
        etna::Device           device,
        etna::RenderPass       renderpass,
        etna::SurfaceKHR       surface,
        uint32_t               image_count,
        etna::SurfaceFormatKHR surface_format,
        etna::Format           depth_format,
        etna::Extent2D         extent,
        etna::PresentModeKHR   present_mode)
    {
        using namespace etna;

        auto usage = ImageUsage::ColorAttachment;

        m_swapchain = device.CreateSwapchainKHR(surface, image_count, surface_format, extent, usage, present_mode);

        auto surface_images = device.GetSwapchainImagesKHR(*m_swapchain);

        for (const auto& color_image : surface_images) {
            auto depth_image = device.CreateImage(
                depth_format,
                extent,
                ImageUsage::DepthStencilAttachment,
                MemoryUsage::GpuOnly,
                ImageTiling::Optimal);

            auto color_view  = device.CreateImageView(color_image, ImageAspect::Color);
            auto depth_view  = device.CreateImageView(*depth_image, ImageAspect::Depth);
            auto framebuffer = device.CreateFramebuffer(renderpass, { *color_view, *depth_view }, extent);

            m_surface_views.push_back(std::move(color_view));
            m_depth_images.push_back(std::move(depth_image));
            m_depth_views.push_back(std::move(depth_view));
            m_framebuffers.push_back(std::move(framebuffer));
        }
    }

    auto ImageCount() const noexcept { return m_surface_views.size(); }
    auto Swapchain() const noexcept { return *m_swapchain; }
    auto Framebuffer(uint32_t image_index) const noexcept { return *m_framebuffers[image_index]; }

  private:
    etna::UniqueSwapchainKHR             m_swapchain;
    std::vector<etna::UniqueImageView2D> m_surface_views;
    std::vector<etna::UniqueImage2D>     m_depth_images;
    std::vector<etna::UniqueImageView2D> m_depth_views;
    std::vector<etna::UniqueFramebuffer> m_framebuffers;
};

void GlfwErrorCallback(int, const char* description)
{
    spdlog::error("GLFW: {}", description);
}

int main()
{
#ifdef NDEBUG
    const bool enable_validation = false;
#else
    const bool enable_validation = true;
#endif

    using namespace etna;

    if (glfwVulkanSupported() == false) {
        throw std::runtime_error("GLFW Vulkan not supported!");
    }

    spdlog::info("GLFW {}", glfwGetVersionString());

    glfwSetErrorCallback(GlfwErrorCallback);

    UniqueInstance instance;
    {
        std::vector<const char*> extensions;
        std::vector<const char*> layers;

        auto count           = 0U;
        auto glfw_extensions = glfwGetRequiredInstanceExtensions(&count);

        extensions.insert(extensions.end(), glfw_extensions, glfw_extensions + count);

        if (enable_validation) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            layers.push_back("VK_LAYER_KHRONOS_validation");
        }

        instance = CreateInstance("Vega", Version{ 0, 1, 0 }, extensions, layers);
    }

    PhysicalDevice gpu;
    {
        auto gpus = instance->EnumeratePhysicalDevices();
        gpu       = gpus.front();

        auto properties = gpu.GetPhysicalDeviceQueueFamilyProperties();
        bool supported  = false;
        for (uint32_t index = 0; index < properties.size(); ++index) {
            supported |= (GLFW_TRUE == glfwGetPhysicalDevicePresentationSupport(*instance, gpu, index));
        }
        if (supported == false) {
            throw std::runtime_error("No gpu queue supports presentation!");
        }
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    auto window = glfwCreateWindow(800, 600, "Vega Viewer", nullptr, nullptr);

    UniqueSurfaceKHR surface;
    {
        VkSurfaceKHR vk_surface{};

        if (VK_SUCCESS != glfwCreateWindowSurface(*instance, window, nullptr, &vk_surface)) {
            throw std::runtime_error("Failed to create window surface!");
        }

        surface.reset(SurfaceKHR(*instance, vk_surface));
    }

    auto [graphics, compute, transfer, presentation] = GetQueueFamilyInfo(gpu, *surface);

    UniqueDevice device;
    {
        auto queue_family_indices = RemoveDuplicates(
            { graphics.family_index, compute.family_index, transfer.family_index, presentation.family_index });

        auto builder = Device::Builder();

        for (auto queue_family_index : queue_family_indices) {
            builder.AddQueue(queue_family_index, 1);
        }

        builder.AddEnabledExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        device = instance->CreateDevice(gpu, builder.state);
    }

    Queues queues;
    {
        queues.graphics     = device->GetQueue(graphics.family_index);
        queues.compute      = device->GetQueue(compute.family_index);
        queues.transfer     = device->GetQueue(transfer.family_index);
        queues.presentation = device->GetQueue(presentation.family_index);
    }

    auto mesh = LoadMesh("C:/Users/slobodan/Documents/Programming/Vega/data/models/suzanne.obj");

    // Copy mesh data to GPU
    UniqueBuffer vertex_buffer;
    UniqueBuffer index_buffer;
    {
        void* data = nullptr;

        auto src_vbo = device->CreateBuffer(mesh.vertices.size(), BufferUsage::TransferSrc, MemoryUsage::CpuOnly);
        auto src_ibo = device->CreateBuffer(mesh.indices.size(), BufferUsage::TransferSrc, MemoryUsage::CpuOnly);

        data = src_vbo->MapMemory();
        memcpy(data, mesh.vertices.data(), mesh.vertices.size());
        src_vbo->UnmapMemory();

        data = src_ibo->MapMemory();
        memcpy(data, mesh.indices.data(), mesh.indices.size());
        src_ibo->UnmapMemory();

        vertex_buffer = device->CreateBuffer(
            mesh.vertices.size(),
            BufferUsage::VertexBuffer | BufferUsage::TransferDst,
            MemoryUsage::GpuOnly);

        index_buffer = device->CreateBuffer(
            mesh.indices.size(),
            BufferUsage::IndexBuffer | BufferUsage::TransferDst,
            MemoryUsage::GpuOnly);

        auto cmd_pool   = device->CreateCommandPool(transfer.family_index, CommandPoolCreate::Transient);
        auto cmd_buffer = cmd_pool->AllocateCommandBuffer();

        cmd_buffer->Begin(CommandBufferUsage::OneTimeSubmit);
        cmd_buffer->CopyBuffer(*src_vbo, *vertex_buffer, mesh.vertices.size());
        cmd_buffer->CopyBuffer(*src_ibo, *index_buffer, mesh.indices.size());
        cmd_buffer->End();

        queues.transfer.Submit(*cmd_buffer);
        device->WaitIdle();
    }

    SurfaceFormatKHR surface_format = FindOptimalSurfaceFormatKHR(
        gpu,
        *surface,
        { SurfaceFormatKHR{ Format::B8G8R8A8Srgb, ColorSpaceKHR::SrgbNonlinear } });

    Format depth_format = FindSupportedFormat(
        gpu,
        { Format::D24UnormS8Uint, Format::D32SfloatS8Uint, Format::D16Unorm },
        ImageTiling::Optimal,
        FormatFeature::DepthStencilAttachment);

    Extent2D extent{ 800, 600 };
    {
        auto capabilities = gpu.GetPhysicalDeviceSurfaceCapabilitiesKHR(*surface);

        if (capabilities.currentExtent.width != UINT32_MAX) {
            extent = capabilities.currentExtent;
        } else {
            extent.width =
                std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            extent.height =
                std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        }
    }

    // Create pipeline renderpass
    UniqueRenderPass renderpass;
    {
        auto builder = RenderPass::Builder();

        auto color_attachment = builder.AddAttachmentDescription(
            surface_format.format,
            AttachmentLoadOp::Clear,
            AttachmentStoreOp::Store,
            ImageLayout::Undefined,
            ImageLayout::PresentSrcKHR);

        auto depth_attachment = builder.AddAttachmentDescription(
            depth_format,
            AttachmentLoadOp::Clear,
            AttachmentStoreOp::DontCare,
            ImageLayout::Undefined,
            ImageLayout::DepthStencilAttachmentOptimal);

        auto color_ref = builder.AddAttachmentReference(color_attachment, ImageLayout::ColorAttachmentOptimal);
        auto depth_ref = builder.AddAttachmentReference(depth_attachment, ImageLayout::DepthStencilAttachmentOptimal);

        auto subpass_builder = builder.GetSubpassBuilder();

        subpass_builder.AddColorAttachment(color_ref);
        subpass_builder.SetDepthStencilAttachment(depth_ref);

        auto subpass_id = builder.AddSubpass(subpass_builder.state);

        builder.AddSubpassDependency(
            SubpassID::External,
            subpass_id,
            PipelineStage::ColorAttachmentOutput,
            PipelineStage::ColorAttachmentOutput,
            Access{},
            Access::ColorAttachmentWrite);

        renderpass = device->CreateRenderPass(builder.state);
    }

    // Create descriptor set layouts
    UniqueDescriptorSetLayout descriptor_set_layout;
    {
        auto builder = DescriptorSetLayout::Builder();
        builder.AddDescriptorSetLayoutBinding(Binding{ 0 }, DescriptorType::UniformBuffer, 1, ShaderStage::Vertex);
        descriptor_set_layout = device->CreateDescriptorSetLayout(builder.state);
    }

    // Create pipeline layout
    UniquePipelineLayout layout;
    {
        auto builder = PipelineLayout::Builder();
        builder.AddDescriptorSetLayout(*descriptor_set_layout);
        layout = device->CreatePipelineLayout(builder.state);
    }

    // Create pipeline
    UniquePipeline pipeline;
    {
        auto builder            = Pipeline::Builder(*layout, *renderpass);
        auto [vs_data, vs_size] = GetResource("shader.vert");
        auto [fs_data, fs_size] = GetResource("shader.frag");
        auto vertex_shader      = device->CreateShaderModule(vs_data, vs_size);
        auto fragment_shader    = device->CreateShaderModule(fs_data, fs_size);
        auto [width, height]    = extent;
        auto viewport           = Viewport{ 0, 0, static_cast<float>(width), static_cast<float>(height), 0, 1 };
        auto scissor            = Rect2D{ Offset2D{ 0, 0 }, Extent2D{ width, height } };

        builder.AddShaderStage(*vertex_shader, ShaderStage::Vertex);
        builder.AddShaderStage(*fragment_shader, ShaderStage::Fragment);
        builder.AddVertexInputBindingDescription(Binding{ 0 }, sizeof(VertexPN));
        builder.AddVertexInputAttributeDescription(
            Location{ 0 },
            Binding{ 0 },
            formatof(VertexPN, position),
            offsetof(VertexPN, position));
        builder.AddVertexInputAttributeDescription(
            Location{ 1 },
            Binding{ 0 },
            formatof(VertexPN, normal),
            offsetof(VertexPN, normal));
        builder.AddViewport(viewport);
        builder.AddScissor(scissor);
        builder.SetDepthState(DepthTest::Enable, DepthWrite::Enable, CompareOp::Less);
        builder.AddColorBlendAttachmentState();

        pipeline = device->CreateGraphicsPipeline(builder.state);
    }

    auto swapchain_manager =
        SwapchainManager(*device, *renderpass, *surface, 3, surface_format, depth_format, extent, PresentModeKHR::Fifo);

    auto image_ready_fences = std::vector<Fence>(swapchain_manager.ImageCount());

    FrameManager frame_manager(*device, graphics.family_index, 2);

    DescriptorManager descriptor_manager(*device, 2, *descriptor_set_layout);

    auto t1 = glfwGetTime();

    float angle = 0;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        auto frame       = frame_manager.NextFrame();
        auto swapchain   = swapchain_manager.Swapchain();
        auto image_index = device->AcquireNextImageKHR(swapchain, frame.image_acquired_semaphore, nullptr).value();

        auto& image_fence = image_ready_fences[image_index];
        if (image_fence != Fence::Null && image_fence != frame.frame_available_fence) {
            device->WaitForFence(image_ready_fences[image_index]);
        }
        image_fence = frame.frame_available_fence;

        auto t2 = glfwGetTime();
        if (t2 - t1 > 0.02) {
            t1 = t2;
            angle += 2;
            if (angle > 360)
                angle -= 360;
        }

        auto eye    = glm::vec3(0, 0, 3 + sinf(glm::radians(angle)));
        auto center = glm::vec3(0, 0, 0);
        auto up     = glm::vec3(0, -1, 0);
        auto model  = glm::rotate(glm::radians(angle), up);
        auto view   = glm::lookAtRH(eye, center, up);
        auto proj   = glm::perspectiveRH(glm::radians(45.f), float(extent.width) / float(extent.height), 0.1f, 1000.0f);
        auto mvp    = MVP{ model, view, proj };

        descriptor_manager.UpdateDescriptorSet(frame.index, mvp);

        auto clear_color       = ClearColor::Transparent;
        auto clear_depth       = ClearDepthStencil::Default;
        auto render_area       = Rect2D{ Offset2D{ 0, 0 }, extent };
        auto wait_semaphores   = { frame.image_acquired_semaphore };
        auto wait_stages       = { PipelineStage::ColorAttachmentOutput };
        auto signal_semaphores = { frame.image_rendered_semaphore };
        auto signal_fence      = frame.frame_available_fence;
        auto framebuffer       = swapchain_manager.Framebuffer(image_index);
        auto descriptor_set    = descriptor_manager.DescriptorSet(frame.index);

        frame.command_buffer.ResetCommandBuffer();

        frame.command_buffer.Begin(CommandBufferUsage::OneTimeSubmit);
        frame.command_buffer.BeginRenderPass(framebuffer, render_area, { clear_color, clear_depth });
        frame.command_buffer.BindPipeline(PipelineBindPoint::Graphics, *pipeline);
        frame.command_buffer.BindVertexBuffers(*vertex_buffer);
        frame.command_buffer.BindIndexBuffer(*index_buffer, IndexType::Uint32);
        frame.command_buffer.BindDescriptorSet(PipelineBindPoint::Graphics, *layout, descriptor_set);
        frame.command_buffer.DrawIndexed(mesh.indices.count(), 1);
        frame.command_buffer.EndRenderPass();
        frame.command_buffer.End();

        queues.graphics.Submit(frame.command_buffer, wait_semaphores, wait_stages, signal_semaphores, signal_fence);

        queues.presentation.QueuePresentKHR(swapchain, image_index, { frame.image_rendered_semaphore });
    }

    device->WaitIdle();

    glfwDestroyWindow(window);

    return 0;
}
