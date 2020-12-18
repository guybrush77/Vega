#include "etna/etna.hpp"

#include "camera.hpp"
#include "gui.hpp"
#include "scene.hpp"
#include "utils/resource.hpp"

BEGIN_DISABLE_WARNINGS

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtx/transform.hpp>
#include <glm/matrix.hpp>

#include <spdlog/spdlog.h>

END_DISABLE_WARNINGS

#include <algorithm>
#include <filesystem>
#include <optional>
#include <unordered_map>
#include <vector>

struct GLFW {
    GLFW()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    }
    ~GLFW() { glfwTerminate(); }
} glfw;

struct ModelTransform final {
    glm::mat4 model;
};

struct CameraTransform final {
    glm::mat4 view;
    glm::mat4 projection;
};

struct Index final {
    Index() noexcept = default;

    constexpr Index(tinyobj::index_t idx) noexcept
        : vertex(static_cast<uint32_t>(idx.vertex_index)), normal(static_cast<uint32_t>(idx.normal_index)),
          texcoord(static_cast<uint32_t>(idx.texcoord_index))
    {}

    struct Hash final {
        constexpr size_t operator()(const Index& index) const noexcept
        {
            size_t hash = 23;

            hash = hash * 31 + index.vertex;
            hash = hash * 31 + index.normal;
            hash = hash * 31 + index.texcoord;

            return hash;
        }
    };

    uint32_t vertex;
    uint32_t normal;
    uint32_t texcoord;
};

constexpr bool operator==(const Index& lhs, const Index& rhs) noexcept
{
    return lhs.vertex == rhs.vertex && lhs.normal == rhs.normal && lhs.texcoord == rhs.texcoord;
}

void LoadObj(ScenePtr scene, std::filesystem::path filepath)
{
    namespace fs = std::filesystem;

    if (false == fs::exists(filepath)) {
        throw std::runtime_error("File does not exist");
    }

    auto parent_dir = fs::path(filepath).parent_path();

    auto attributes = tinyobj::attrib_t{};
    auto shapes     = std::vector<tinyobj::shape_t>{};
    auto materials  = std::vector<tinyobj::material_t>{};
    auto warning    = std::string{};
    auto err        = std::string{};

    bool success = tinyobj::LoadObj(
        &attributes,
        &shapes,
        &materials,
        &warning,
        &err,
        filepath.string().c_str(),
        parent_dir.string().c_str(),
        true,
        false);

    if (success == false) {
        throw std::runtime_error("Failed to load file");
    }

    auto material_root     = scene->GetMaterialRootPtr();
    auto material_class    = material_root->AddMaterialClassNode();
    auto material_instance = material_class->AddMaterialInstanceNode();

    auto geometry_root = scene->GetGeometryRootPtr();

    auto index_map = std::unordered_map<Index, uint32_t, Index::Hash>{};
    auto vertices  = std::vector<VertexPN>{};
    auto indices   = std::vector<uint32_t>{};
    auto meshes    = std::vector<Mesh>{};

    for (const auto& shape : shapes) {
        auto num_indices = shape.mesh.indices.size();
        auto aabb        = AABB{ { FLT_MAX, FLT_MAX, FLT_MAX }, { FLT_MIN, FLT_MIN, FLT_MIN } };

        assert(num_indices % 3 == 0);

        for (size_t i = 0; i < num_indices; i += 3) {
            for (std::size_t j = 0; j < 3; ++j) {
                auto index = Index(shape.mesh.indices[i + j]);
                if (auto it = index_map.find(index); it != index_map.end()) {
                    indices.push_back(it->second);
                } else {
                    auto position_idx = static_cast<size_t>(3) * index.vertex;
                    auto normal_idx   = static_cast<size_t>(3) * index.normal;

                    auto vertex = VertexPN{

                        .position = { attributes.vertices[position_idx + 0],
                                      attributes.vertices[position_idx + 1],
                                      attributes.vertices[position_idx + 2] },

                        .normal = { attributes.normals[normal_idx + 0],
                                    attributes.normals[normal_idx + 1],
                                    attributes.normals[normal_idx + 2] }
                    };

                    vertices.push_back(vertex);

                    aabb.min = { std::min(aabb.min.x, vertex.position.x),
                                 std::min(aabb.min.y, vertex.position.y),
                                 std::min(aabb.min.z, vertex.position.z) };

                    aabb.max = { std::max(aabb.max.x, vertex.position.x),
                                 std::max(aabb.max.y, vertex.position.y),
                                 std::max(aabb.max.z, vertex.position.z) };

                    auto idx         = static_cast<uint32_t>(vertices.size() - 1);
                    index_map[index] = idx;

                    indices.push_back(idx);
                }
            }
        }

        auto mesh = scene->CreateMesh(aabb, std::move(vertices), std::move(indices));

        mesh->SetProperty("name", shape.name);

        auto mesh_instance = geometry_root->AddInstanceNode(mesh, material_instance);

        mesh_instance->SetProperty("name", shape.name);
    }
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

constexpr float ComputeAspect(etna::Extent2D extent)
{
    return etna::narrow_cast<float>(extent.width) / etna::narrow_cast<float>(extent.height);
}

VkBool32 VulkanDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT vk_message_severity,
    VkDebugUtilsMessageTypeFlagsEXT,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void*)
{
    auto message_severity = static_cast<etna::DebugUtilsMessageSeverity>(vk_message_severity);

    switch (message_severity) {
    case etna::DebugUtilsMessageSeverity::Verbose: spdlog::debug(callback_data->pMessage); break;
    case etna::DebugUtilsMessageSeverity::Info: spdlog::info(callback_data->pMessage); break;
    case etna::DebugUtilsMessageSeverity::Warning: spdlog::warn(callback_data->pMessage); break;
    case etna::DebugUtilsMessageSeverity::Error: spdlog::error(callback_data->pMessage); break;
    default:
        spdlog::warn("Vulkan message callback message severity not recognized");
        spdlog::error(callback_data->pMessage);
        break;
    }

    return VK_FALSE;
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
        throw std::runtime_error("Failed to detect GPU graphics queue!");
    }

    if (dedicated_compute.has_value()) {
        compute = dedicated_compute;
    } else if (mixed_compute.has_value()) {
        compute = mixed_compute;
    } else if (graphics_compute.has_value()) {
        compute = graphics_compute;
    }

    if (false == compute.has_value()) {
        throw std::runtime_error("Failed to detect GPU compute queue!");
    }

    if (dedicated_transfer.has_value()) {
        transfer = dedicated_transfer;
    } else if (mixed_transfer.has_value()) {
        transfer = mixed_transfer;
    } else if (graphics_transfer.has_value()) {
        transfer = graphics_transfer;
    }

    if (false == transfer.has_value()) {
        throw std::runtime_error("Failed to detect GPU transfer queue!");
    }

    if (graphics_presentation.has_value()) {
        presentation = graphics_presentation;
    } else if (mixed_presentation.has_value()) {
        presentation = mixed_presentation;
    }

    if (false == presentation.has_value()) {
        throw std::runtime_error("Failed to detect GPU presentation queue!");
    }

    return { graphics.value(), compute.value(), transfer.value(), presentation.value() };
}

etna::SurfaceFormatKHR FindOptimalSurfaceFormatKHR(
    etna::PhysicalDevice                          gpu,
    etna::SurfaceKHR                              surface,
    std::initializer_list<etna::SurfaceFormatKHR> preffered_formats)
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
    etna::PhysicalDevice                gpu,
    std::initializer_list<etna::Format> candidate_formats,
    etna::ImageTiling                   tiling,
    etna::FormatFeature                 required_features)
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
}

struct FrameInfo {
    uint32_t index;
    struct {
        etna::CommandBuffer draw;
        etna::CommandBuffer gui;
    } cmd_buffers;
    struct {
        etna::Semaphore image_acquired;
        etna::Semaphore draw_completed;
        etna::Semaphore gui_completed;
    } semaphores;
    struct {
        etna::Fence image_ready;
    } fence;
};

class FrameManager {
  public:
    FrameManager(etna::Device device, uint32_t queue_family_index, uint32_t frame_count)
        : m_device(device), m_frame_count(frame_count), m_next_frame(0)
    {
        m_command_pool = device.CreateCommandPool(queue_family_index, etna::CommandPoolCreate::ResetCommandBuffer);

        for (uint32_t frame_index = 0; frame_index < frame_count; ++frame_index) {
            m_draw_command_buffers.push_back(m_command_pool->AllocateCommandBuffer());
            m_gui_command_buffers.push_back(m_command_pool->AllocateCommandBuffer());
            m_image_acquired_sempahores.push_back(device.CreateSemaphore());
            m_draw_completed_sempahores.push_back(device.CreateSemaphore());
            m_gui_completed_sempahores.push_back(device.CreateSemaphore());
            m_frame_available_fences.push_back(device.CreateFence(etna::FenceCreate::Signaled));
            m_frame_info.push_back(FrameInfo{
                frame_index,
                { *m_draw_command_buffers.back(), *m_gui_command_buffers.back() },
                { *m_image_acquired_sempahores.back(),
                  *m_draw_completed_sempahores.back(),
                  *m_gui_completed_sempahores.back() },
                { *m_frame_available_fences.back() },
            });
        }
    }

    const FrameInfo NextFrame()
    {
        auto frame_index       = m_next_frame;
        auto image_ready_fence = m_frame_info[frame_index].fence.image_ready;

        m_next_frame = (m_next_frame + 1) % m_frame_count;

        m_device.WaitForFence(image_ready_fence);
        m_device.ResetFence(image_ready_fence);

        return m_frame_info[frame_index];
    }

  private:
    etna::Device                           m_device;
    etna::UniqueCommandPool                m_command_pool;
    std::vector<etna::UniqueCommandBuffer> m_draw_command_buffers;
    std::vector<etna::UniqueCommandBuffer> m_gui_command_buffers;
    std::vector<etna::UniqueSemaphore>     m_image_acquired_sempahores;
    std::vector<etna::UniqueSemaphore>     m_draw_completed_sempahores;
    std::vector<etna::UniqueSemaphore>     m_gui_completed_sempahores;
    std::vector<etna::UniqueFence>         m_frame_available_fences;
    std::vector<FrameInfo>                 m_frame_info;
    uint32_t                               m_frame_count;
    uint32_t                               m_next_frame;
};

class DescriptorManager {
  public:
    DescriptorManager() noexcept = default;

    DescriptorManager(const DescriptorManager&) = delete;
    DescriptorManager& operator=(const DescriptorManager&) = delete;

    DescriptorManager(DescriptorManager&&) = default;
    DescriptorManager& operator=(DescriptorManager&&) = default;

    DescriptorManager(
        etna::Device                      device,
        uint32_t                          num_frames,
        etna::DescriptorSetLayout         descriptor_set_layout,
        const etna::PhysicalDeviceLimits& gpu_limits)
        : m_device(device), m_descriptor_set_layout(descriptor_set_layout)
    {
        using namespace etna;

        m_offset_multiplier = sizeof(glm::mat4);

        if (auto min_alignment = gpu_limits.minUniformBufferOffsetAlignment; min_alignment > 0) {
            m_offset_multiplier = (m_offset_multiplier + min_alignment - 1) & ~(min_alignment - 1);
        }

        m_descriptor_pool =
            device.CreateDescriptorPool({ DescriptorPoolSize{ DescriptorType::UniformBuffer, num_frames } });

        auto descriptor_sets = m_descriptor_pool->AllocateDescriptorSets(num_frames, descriptor_set_layout);

        auto model_buffers = device.CreateBuffers(
            num_frames,
            kMaxTransforms * m_offset_multiplier,
            BufferUsage::UniformBuffer,
            MemoryUsage::CpuToGpu);

        auto camera_buffers = device.CreateBuffers(
            num_frames,
            sizeof(CameraTransform),
            BufferUsage::UniformBuffer,
            MemoryUsage::CpuToGpu);

        auto write_descriptor_sets = std::vector<WriteDescriptorSet>();

        m_frame_states.reserve(num_frames);
        write_descriptor_sets.reserve(num_frames);

        for (size_t i = 0; i < num_frames; ++i) {
            auto model_buffer_memory  = static_cast<std::byte*>(model_buffers[i]->MapMemory());
            auto camera_buffer_memory = static_cast<std::byte*>(camera_buffers[i]->MapMemory());

            auto frame_state = FrameState{

                descriptor_sets[i],
                { std::move(model_buffers[i]), model_buffer_memory },
                { std::move(camera_buffers[i]), camera_buffer_memory }
            };

            m_frame_states.push_back(std::move(frame_state));

            write_descriptor_sets.emplace_back(descriptor_sets[i], Binding{ 0 }, DescriptorType::UniformBufferDynamic);
            write_descriptor_sets.back().AddBuffer(*m_frame_states[i].model.buffer);

            write_descriptor_sets.emplace_back(descriptor_sets[i], Binding{ 1 }, DescriptorType::UniformBuffer);
            write_descriptor_sets.back().AddBuffer(*m_frame_states[i].camera.buffer);
        }

        m_device.UpdateDescriptorSets(write_descriptor_sets);
    }

    ~DescriptorManager() noexcept
    {
        for (auto& frame_state : m_frame_states) {
            frame_state.model.buffer->UnmapMemory();
            frame_state.camera.buffer->UnmapMemory();
        }
    }

    auto DescriptorSet(size_t frame_index) const noexcept { return m_frame_states[frame_index].descriptor_set; }
    auto DescriptorSetLayout() const noexcept { return m_descriptor_set_layout; }

    uint32_t Set(size_t frame_index, size_t transform_index, const ModelTransform& model_transform) noexcept
    {
        auto& frame_state = m_frame_states[frame_index];

        auto offset = transform_index * m_offset_multiplier;

        memcpy(frame_state.model.mapped_memory + offset, &model_transform, m_offset_multiplier);

        return etna::narrow_cast<uint32_t>(offset);
    }

    void Set(size_t frame_index, const CameraTransform& camera_transform) noexcept
    {
        auto& frame_state = m_frame_states[frame_index];

        memcpy(frame_state.camera.mapped_memory, &camera_transform, sizeof(camera_transform));
    }

    void UpdateDescriptorSet(size_t frame_index)
    {
        using namespace etna;

        auto& frame_state = m_frame_states[frame_index];

        frame_state.model.buffer->FlushMappedMemoryRanges({ MappedMemoryRange{} });
        frame_state.camera.buffer->FlushMappedMemoryRanges({ MappedMemoryRange{} });
    }

  private:
    static constexpr int kMaxTransforms = 64;

    struct FrameState final {
        etna::DescriptorSet descriptor_set;

        struct Model final {
            etna::UniqueBuffer buffer{};
            std::byte*         mapped_memory{};
        } model;

        struct Camera final {
            etna::UniqueBuffer buffer{};
            std::byte*         mapped_memory{};
        } camera;
    };

    etna::Device               m_device;
    etna::DescriptorSetLayout  m_descriptor_set_layout;
    etna::UniqueDescriptorPool m_descriptor_pool;
    std::vector<FrameState>    m_frame_states;
    etna::DeviceSize           m_offset_multiplier;
};

struct FramebufferInfo {
    etna::Framebuffer draw;
    etna::Framebuffer gui;
    etna::Extent2D    extent;
};

class SwapchainManager {
  public:
    SwapchainManager(
        etna::Device           device,
        etna::RenderPass       renderpass,
        etna::RenderPass       gui_renderpass,
        etna::SurfaceKHR       surface,
        uint32_t               min_image_count,
        etna::SurfaceFormatKHR surface_format,
        etna::Format           depth_format,
        etna::Extent2D         extent,
        etna::Queue            presentation_queue,
        etna::PresentModeKHR   present_mode)
        : m_device(device), m_presentation_queue(presentation_queue), m_extent(extent),
          m_min_image_count(min_image_count)
    {
        using namespace etna;

        auto usage = ImageUsage::ColorAttachment;

        m_swapchain = device.CreateSwapchainKHR(surface, min_image_count, surface_format, extent, usage, present_mode);

        auto surface_images = device.GetSwapchainImagesKHR(*m_swapchain);

        for (const auto& color_image : surface_images) {
            auto depth_image = device.CreateImage(
                depth_format,
                extent,
                ImageUsage::DepthStencilAttachment,
                MemoryUsage::GpuOnly,
                ImageTiling::Optimal);

            auto color_view      = device.CreateImageView(color_image, ImageAspect::Color);
            auto depth_view      = device.CreateImageView(*depth_image, ImageAspect::Depth);
            auto framebuffer     = device.CreateFramebuffer(renderpass, { *color_view, *depth_view }, extent);
            auto gui_framebuffer = device.CreateFramebuffer(gui_renderpass, { *color_view }, extent);

            m_surface_views.push_back(std::move(color_view));
            m_depth_images.push_back(std::move(depth_image));
            m_depth_views.push_back(std::move(depth_view));
            m_framebuffers.push_back(std::move(framebuffer));
            m_gui_framebuffers.push_back(std::move(gui_framebuffer));
        }
    }

    auto AcquireNextImage(etna::Semaphore semaphore, etna::Fence fence = {}) -> etna::Return<uint32_t>
    {
        return m_device.AcquireNextImageKHR(*m_swapchain, semaphore, fence);
    }

    auto QueuePresent(uint32_t image_index, std::initializer_list<etna::Semaphore> wait_semaphores) -> etna::Result
    {
        return m_presentation_queue.QueuePresentKHR(*m_swapchain, image_index, wait_semaphores);
    }

    uint32_t ImageCount() const noexcept { return etna::narrow_cast<uint32_t>(m_surface_views.size()); }

    uint32_t MinImageCount() const noexcept { return m_min_image_count; }

    FramebufferInfo GetFramebufferInfo(uint32_t image_index) const noexcept
    {
        return FramebufferInfo{ *m_framebuffers[image_index], *m_gui_framebuffers[image_index], m_extent };
    }

  private:
    etna::UniqueSwapchainKHR             m_swapchain;
    std::vector<etna::UniqueImageView2D> m_surface_views;
    std::vector<etna::UniqueImage2D>     m_depth_images;
    std::vector<etna::UniqueImageView2D> m_depth_views;
    std::vector<etna::UniqueFramebuffer> m_framebuffers;
    std::vector<etna::UniqueFramebuffer> m_gui_framebuffers;
    etna::Device                         m_device;
    etna::Queue                          m_presentation_queue;
    etna::Extent2D                       m_extent;
    uint32_t                             m_min_image_count;
};

struct MeshRecord final {
    struct Vertex final {
        etna::Buffer buffer;
        uint32_t     size;
        uint32_t     count;
    } vertices;
    struct Indexfinal {
        etna::Buffer buffer;
        uint32_t     size;
        uint32_t     count;
    } indices;
};

class MeshStore {
  public:
    MeshStore(etna::Device device) : m_device(device) {}

    MeshStore(const MeshStore&) = delete;
    MeshStore& operator=(const MeshStore&) = delete;

    MeshStore(MeshStore&&) = default;
    MeshStore& operator=(MeshStore&&) = default;

    bool Add(MeshPtr mesh)
    {
        using namespace etna;

        if (m_cpu_buffers.count(mesh)) {
            return false;
        }

        auto vertices_size  = mesh->Vertices().Size();
        auto vertices_data  = mesh->Vertices().Data();
        auto vertices_count = mesh->Vertices().Count();

        auto indices_size  = mesh->Indices().Size();
        auto indices_data  = mesh->Indices().Data();
        auto indices_count = mesh->Indices().Count();

        auto cpu_vertex_buffer = m_device.CreateBuffer(vertices_size, BufferUsage::TransferSrc, MemoryUsage::CpuOnly);
        auto cpu_index_buffer  = m_device.CreateBuffer(indices_size, BufferUsage::TransferSrc, MemoryUsage::CpuOnly);

        auto vertex_buffer_data = cpu_vertex_buffer->MapMemory();
        memcpy(vertex_buffer_data, vertices_data, vertices_size);
        cpu_vertex_buffer->UnmapMemory();

        auto index_buffer_data = cpu_index_buffer->MapMemory();
        memcpy(index_buffer_data, indices_data, indices_size);
        cpu_index_buffer->UnmapMemory();

        auto cpu_buffers = MeshRecordPrivate{ { std::move(cpu_vertex_buffer), vertices_size, vertices_count },
                                              { std::move(cpu_index_buffer), indices_size, indices_count } };

        auto rv = m_cpu_buffers.insert({ mesh, std::move(cpu_buffers) });

        return rv.second;
    }

    void Upload(etna::Queue transfer_queue, uint32_t transfer_queue_family_index)
    {
        using namespace etna;

        auto cmd_pool   = m_device.CreateCommandPool(transfer_queue_family_index, CommandPoolCreate::Transient);
        auto cmd_buffer = cmd_pool->AllocateCommandBuffer();

        cmd_buffer->Begin(CommandBufferUsage::OneTimeSubmit);

        for (const auto& [mesh_id, cpu_mesh] : m_cpu_buffers) {
            auto vertices_size  = cpu_mesh.vertices.size;
            auto vertices_count = cpu_mesh.vertices.count;
            auto indices_size   = cpu_mesh.indices.size;
            auto indices_count  = cpu_mesh.indices.count;

            auto gpu_vertex_buffer = m_device.CreateBuffer(
                vertices_size,
                BufferUsage::VertexBuffer | BufferUsage::TransferDst,
                MemoryUsage::GpuOnly);

            auto gpu_index_buffer = m_device.CreateBuffer(
                indices_size,
                BufferUsage::IndexBuffer | BufferUsage::TransferDst,
                MemoryUsage::GpuOnly);

            auto gpu_buffers = MeshRecordPrivate{ { std::move(gpu_vertex_buffer), vertices_size, vertices_count },
                                                  { std::move(gpu_index_buffer), indices_size, indices_count } };

            auto [it, success] = m_gpu_buffers.insert({ mesh_id, std::move(gpu_buffers) });

            const auto& gpu_mesh = it->second;

            cmd_buffer->CopyBuffer(*cpu_mesh.vertices.buffer, *gpu_mesh.vertices.buffer, vertices_size);
            cmd_buffer->CopyBuffer(*cpu_mesh.indices.buffer, *gpu_mesh.indices.buffer, indices_size);
        }

        cmd_buffer->End();

        transfer_queue.Submit(*cmd_buffer);

        m_device.WaitIdle();

        m_cpu_buffers.clear();
    }

    auto Get(MeshPtr mesh)
    {
        if (auto it = m_gpu_buffers.find(mesh); it != m_gpu_buffers.end()) {
            const auto& buffers = it->second;
            return MeshRecord{ { *buffers.vertices.buffer, buffers.vertices.size, buffers.vertices.count },
                               { *buffers.indices.buffer, buffers.indices.size, buffers.indices.count } };
        }
        return MeshRecord{};
    }

  private:
    struct MeshRecordPrivate final {
        struct Vertex final {
            etna::UniqueBuffer buffer;
            uint32_t           size;
            uint32_t           count;
        } vertices;
        struct Indexfinal {
            etna::UniqueBuffer buffer;
            uint32_t           size;
            uint32_t           count;
        } indices;
    };

    etna::Device m_device;

    std::unordered_map<MeshPtr, MeshRecordPrivate> m_cpu_buffers;
    std::unordered_map<MeshPtr, MeshRecordPrivate> m_gpu_buffers;
};

class RenderContext {
  public:
    enum Status { WindowClosed, SwapchainOutOfDate };
    enum class MouseLook { None, Orbit, Zoom, Track };

    RenderContext(
        etna::Device         device,
        etna::Queue          graphics_queue,
        etna::Pipeline       pipeline,
        etna::PipelineLayout pipeline_layout,
        GLFWwindow*          window,
        SwapchainManager*    swapchain_manager,
        FrameManager*        frame_manager,
        DescriptorManager*   descriptor_manager,
        Gui*                 gui,
        Camera*              camera,
        MeshStore*           mesh_store,
        Scene*               scene)
        : m_device(device), m_graphics_queue(graphics_queue), m_pipeline(pipeline), m_pipeline_layout(pipeline_layout),
          m_window(window), m_swapchain_manager(swapchain_manager), m_frame_manager(frame_manager),
          m_descriptor_manager(descriptor_manager), m_gui(gui), m_camera(camera), m_mesh_store(mesh_store),
          m_scene(scene)
    {}

    void ProcessUserInput()
    {
        auto mouse_state  = m_gui->GetMouseState();
        bool is_scrolling = mouse_state.scroll.y != 0;

        if (mouse_state.buttons.IsNonePressed() && !is_scrolling) {
            if (m_mouse_look != MouseLook::None) {
                glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
            m_is_any_window_hovered = false;
            m_mouse_look            = MouseLook::None;
            return;
        }

        if (m_mouse_look == MouseLook::None) {
            if (m_is_any_window_hovered || m_gui->IsAnyWindowHovered()) {
                m_is_any_window_hovered = true;
                return;
            }
            if (mouse_state.buttons.left.is_pressed) {
                m_mouse_look = MouseLook::Orbit;
            } else if (mouse_state.buttons.right.is_pressed) {
                m_mouse_look = MouseLook::Track;
            } else if (mouse_state.buttons.middle.is_pressed) {
                m_mouse_look = MouseLook::Zoom;
            } else {
                constexpr auto scroll_sensitivity = 6;
                m_camera->Zoom(scroll_sensitivity * mouse_state.scroll.y);
                return;
            }
            glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }

        if (m_mouse_look == MouseLook::Orbit) {
            auto rot_x = Degrees(mouse_state.cursor.delta.x);
            auto rot_y = Degrees(mouse_state.cursor.delta.y);
            m_camera->Orbit(rot_y, rot_x);
        } else if (m_mouse_look == MouseLook::Track) {
            m_camera->Track(mouse_state.cursor.delta.x, mouse_state.cursor.delta.y);
        } else if (m_mouse_look == MouseLook::Zoom) {
            m_camera->Zoom(mouse_state.cursor.delta.y);
        }
    }

    Status StartRenderLoop()
    {
        using namespace etna;

        auto image_ready_fences = std::vector<Fence>(m_swapchain_manager->ImageCount());

        while (!glfwWindowShouldClose(m_window)) {
            glfwPollEvents();

            auto frame       = m_frame_manager->NextFrame();
            auto image_index = uint32_t{};

            if (auto next_image = m_swapchain_manager->AcquireNextImage(frame.semaphores.image_acquired); next_image) {
                image_index = next_image.value();
                if (image_ready_fences[image_index] != Fence::Null &&
                    image_ready_fences[image_index] != frame.fence.image_ready) {
                    m_device.WaitForFence(image_ready_fences[image_index]);
                }
                image_ready_fences[image_index] = frame.fence.image_ready;
            } else if (next_image.result() == Result::ErrorOutOfDateKHR) {
                return SwapchainOutOfDate;
            } else {
                throw std::runtime_error("AcquireNextImage failed!");
            }

            auto framebuffers = m_swapchain_manager->GetFramebufferInfo(image_index);

            ProcessUserInput();

            auto draw_list = m_scene->GetDrawList();
            auto extent    = framebuffers.extent;

            auto camera_transform = CameraTransform{ m_camera->GetViewMatrix(), m_camera->GetPerspectiveMatrix() };

            m_descriptor_manager->Set(frame.index, camera_transform);

            auto clear_color    = ClearColor::Transparent;
            auto clear_depth    = ClearDepthStencil::Default;
            auto render_area    = Rect2D{ Offset2D{ 0, 0 }, extent };
            auto framebuffer    = framebuffers.draw;
            auto descriptor_set = m_descriptor_manager->DescriptorSet(frame.index);
            auto width          = narrow_cast<float>(extent.width);
            auto height         = narrow_cast<float>(extent.height);
            auto viewport       = Viewport{ 0, height, width, -height, 0, 1 };
            auto scissor        = Rect2D{ Offset2D{ 0, 0 }, extent };

            frame.cmd_buffers.draw.ResetCommandBuffer();
            frame.cmd_buffers.draw.Begin(CommandBufferUsage::OneTimeSubmit);
            frame.cmd_buffers.draw.BeginRenderPass(framebuffer, render_area, { clear_color, clear_depth });
            frame.cmd_buffers.draw.BindPipeline(PipelineBindPoint::Graphics, m_pipeline);
            frame.cmd_buffers.draw.SetViewport(viewport);
            frame.cmd_buffers.draw.SetScissor(scissor);

            for (size_t i = 0; i < draw_list.size(); ++i) {
                auto model_transform = ModelTransform{ *draw_list[i].transform };
                auto offset          = m_descriptor_manager->Set(frame.index, i, model_transform);
                auto mesh            = m_mesh_store->Get(draw_list[i].mesh);

                frame.cmd_buffers.draw.BindVertexBuffers(mesh.vertices.buffer);
                frame.cmd_buffers.draw.BindIndexBuffer(mesh.indices.buffer, IndexType::Uint32); // TODO: buffer type?
                frame.cmd_buffers.draw
                    .BindDescriptorSet(PipelineBindPoint::Graphics, m_pipeline_layout, descriptor_set, { offset });
                frame.cmd_buffers.draw.DrawIndexed(mesh.indices.count, 1);
            }

            frame.cmd_buffers.draw.EndRenderPass();
            frame.cmd_buffers.draw.End();

            m_descriptor_manager->UpdateDescriptorSet(frame.index);

            m_graphics_queue.Submit(
                frame.cmd_buffers.draw,
                { frame.semaphores.image_acquired },
                { PipelineStage::ColorAttachmentOutput },
                { frame.semaphores.draw_completed },
                {});

            m_gui->Draw(
                frame.cmd_buffers.gui,
                framebuffers.gui,
                frame.semaphores.draw_completed,
                frame.semaphores.gui_completed,
                frame.fence.image_ready);

            m_swapchain_manager->QueuePresent(image_index, { frame.semaphores.gui_completed });
        }

        return WindowClosed;
    }

  private:
    etna::Device         m_device;
    etna::Queue          m_graphics_queue;
    etna::Pipeline       m_pipeline;
    etna::PipelineLayout m_pipeline_layout;
    GLFWwindow*          m_window                = nullptr;
    SwapchainManager*    m_swapchain_manager     = nullptr;
    FrameManager*        m_frame_manager         = nullptr;
    DescriptorManager*   m_descriptor_manager    = nullptr;
    Gui*                 m_gui                   = nullptr;
    Camera*              m_camera                = nullptr;
    MeshStore*           m_mesh_store            = nullptr;
    Scene*               m_scene                 = nullptr;
    MouseLook            m_mouse_look            = MouseLook::None;
    bool                 m_is_any_window_hovered = false;
};

struct GlfwWindowDeleter {
    void operator()(GLFWwindow* window) { glfwDestroyWindow(window); }
};

using GlfwWindow = std::unique_ptr<GLFWwindow, GlfwWindowDeleter>;

void GlfwErrorCallback(int, const char* description)
{
    spdlog::error("GLFW: {}", description);
}

GlfwWindow CreateWindow(const char* window_name)
{
    GLFWmonitor* primary_monitor = glfwGetPrimaryMonitor();

    int window_width  = 0;
    int window_height = 0;
    int window_pos_x  = 0;
    int window_pos_y  = 0;
    {
        int xpos{}, ypos{}, width{}, height{};
        glfwGetMonitorWorkarea(primary_monitor, &xpos, &ypos, &width, &height);
        window_width  = width * 3 / 4;
        window_height = height * 3 / 4;
        window_pos_x  = (width - window_width) / 2;
        window_pos_y  = (height - window_height) / 2;
    }

    auto window = glfwCreateWindow(window_width, window_height, window_name, nullptr, nullptr);
    if (!window) {
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwSetWindowPos(window, window_pos_x, window_pos_y);

    return GlfwWindow(window);
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

        instance = CreateInstance(
            "Vega",
            Version{ 0, 1, 0 },
            extensions,
            layers,
            VulkanDebugCallback,
            DebugUtilsMessageSeverity::Warning | DebugUtilsMessageSeverity::Error,
            DebugUtilsMessageType::General | DebugUtilsMessageType::Performance | DebugUtilsMessageType::Validation);
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

    auto gpu_properties = gpu.GetPhysicalDeviceProperties();

    auto window = CreateWindow("Vega Viewer");

    UniqueSurfaceKHR surface;
    {
        VkSurfaceKHR vk_surface{};

        if (VK_SUCCESS != glfwCreateWindowSurface(*instance, window.get(), nullptr, &vk_surface)) {
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

    auto queues = Queues{};
    {
        queues.graphics     = device->GetQueue(graphics.family_index);
        queues.compute      = device->GetQueue(compute.family_index);
        queues.transfer     = device->GetQueue(transfer.family_index);
        queues.presentation = device->GetQueue(presentation.family_index);
    }

    auto scene = Scene();

    LoadObj(&scene, "../../../data/models/suzanne.obj");

    SurfaceFormatKHR surface_format = FindOptimalSurfaceFormatKHR(
        gpu,
        *surface,
        { SurfaceFormatKHR{ Format::B8G8R8A8Srgb, ColorSpaceKHR::SrgbNonlinear } });

    Format depth_format = FindSupportedFormat(
        gpu,
        { Format::D24UnormS8Uint, Format::D32SfloatS8Uint, Format::D16Unorm },
        ImageTiling::Optimal,
        FormatFeature::DepthStencilAttachment);

    Extent2D extent{};
    {
        int width{}, height{};
        glfwGetWindowSize(window.get(), &width, &height);
        extent = Extent2D{ narrow_cast<uint32_t>(width), narrow_cast<uint32_t>(height) };

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

    // Aspect
    auto aspect = ComputeAspect(extent);

    // Create pipeline renderpass
    UniqueRenderPass renderpass;
    {
        auto builder = RenderPass::Builder();

        auto color_attachment = builder.AddAttachmentDescription(
            surface_format.format,
            AttachmentLoadOp::Clear,
            AttachmentStoreOp::Store,
            ImageLayout::Undefined,
            ImageLayout::ColorAttachmentOptimal);

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

    // Create gui pipeline renderpass
    UniqueRenderPass gui_renderpass;
    {
        auto builder = RenderPass::Builder();

        auto color_attachment = builder.AddAttachmentDescription(
            surface_format.format,
            AttachmentLoadOp::Load,
            AttachmentStoreOp::Store,
            ImageLayout::ColorAttachmentOptimal,
            ImageLayout::PresentSrcKHR);

        auto color_ref = builder.AddAttachmentReference(color_attachment, ImageLayout::ColorAttachmentOptimal);

        auto subpass_builder = builder.GetSubpassBuilder();

        subpass_builder.AddColorAttachment(color_ref);

        auto subpass_id = builder.AddSubpass(subpass_builder.state);

        builder.AddSubpassDependency(
            SubpassID::External,
            subpass_id,
            PipelineStage::ColorAttachmentOutput,
            PipelineStage::ColorAttachmentOutput,
            Access::ColorAttachmentRead,
            Access::ColorAttachmentWrite);

        gui_renderpass = device->CreateRenderPass(builder.state);
    }

    // Create descriptor set layouts
    UniqueDescriptorSetLayout descriptor_set_layout;
    {
        auto builder = DescriptorSetLayout::Builder();

        builder
            .AddDescriptorSetLayoutBinding(Binding{ 0 }, DescriptorType::UniformBufferDynamic, 1, ShaderStage::Vertex);
        builder.AddDescriptorSetLayoutBinding(Binding{ 1 }, DescriptorType::UniformBuffer, 1, ShaderStage::Vertex);

        descriptor_set_layout = device->CreateDescriptorSetLayout(builder.state);
    }

    // Create pipeline pipeline_layout
    UniquePipelineLayout pipeline_layout;
    {
        auto builder = PipelineLayout::Builder();
        builder.AddDescriptorSetLayout(*descriptor_set_layout);
        pipeline_layout = device->CreatePipelineLayout(builder.state);
    }

    // Create pipeline
    UniquePipeline pipeline;
    {
        auto builder            = Pipeline::Builder(*pipeline_layout, *renderpass);
        auto [vs_data, vs_size] = GetResource("shaders/shader.vert");
        auto [fs_data, fs_size] = GetResource("shaders/shader.frag");
        auto vertex_shader      = device->CreateShaderModule(vs_data, vs_size);
        auto fragment_shader    = device->CreateShaderModule(fs_data, fs_size);
        auto width              = narrow_cast<float>(extent.width);
        auto height             = narrow_cast<float>(extent.height);
        auto viewport           = Viewport{ 0, height, width, -height, 0, 1 };
        auto scissor            = Rect2D{ Offset2D{ 0, 0 }, Extent2D{ extent } };

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
        builder.AddDynamicStates({ DynamicState::Viewport, DynamicState::Scissor });
        builder.SetDepthState(DepthTest::Enable, DepthWrite::Enable, CompareOp::Less);
        builder.AddColorBlendAttachmentState();

        pipeline = device->CreateGraphicsPipeline(builder.state);
    }

    auto draw_list = scene.GetDrawList();

    auto mesh_store = MeshStore(*device);

    for (DrawRecord draw_record : draw_list) {
        mesh_store.Add(draw_record.mesh);
    }

    mesh_store.Upload(queues.transfer, transfer.family_index);

    uint32_t image_count = 3;
    uint32_t frame_count = 2;

    auto descriptor_manager = DescriptorManager(*device, frame_count, *descriptor_set_layout, gpu_properties.limits);

    auto camera = Camera::Create(
        Orientation::RightHanded,
        Forward{ Axis::PositiveY },
        Up{ Axis::PositiveZ },
        ObjectView::Front,
        { { -1, -1, -1 }, { 1, 1, 1 } }, // TODO: get aabb from DrawList
        45_deg,
        aspect);

    auto gui =
        Gui(*instance,
            gpu,
            *device,
            graphics.family_index,
            queues.graphics,
            *gui_renderpass,
            window.get(),
            extent,
            image_count,
            image_count);

    gui.AddWindow<CameraWindow>(&camera);

    bool running = true;

    while (running) {
        auto swapchain_manager = SwapchainManager(
            *device,
            *renderpass,
            *gui_renderpass,
            *surface,
            image_count,
            surface_format,
            depth_format,
            extent,
            queues.presentation,
            PresentModeKHR::Fifo);

        FrameManager frame_manager(*device, graphics.family_index, frame_count);

        auto render_context = RenderContext(
            *device,
            queues.graphics,
            *pipeline,
            *pipeline_layout,
            window.get(),
            &swapchain_manager,
            &frame_manager,
            &descriptor_manager,
            &gui,
            &camera,
            &mesh_store,
            &scene);

        auto result = render_context.StartRenderLoop();

        device->WaitIdle();

        switch (result) {
        case RenderContext::WindowClosed: {
            running = false;
            break;
        }
        case RenderContext::SwapchainOutOfDate: {
            int width  = 0;
            int height = 0;
            while (width == 0 && height == 0) {
                glfwGetWindowSize(window.get(), &width, &height);
                if (width == 0 && height == 0) {
                    glfwWaitEvents();
                }
            }
            extent.width  = narrow_cast<uint32_t>(width);
            extent.height = narrow_cast<uint32_t>(height);
            gui.UpdateViewport(extent, swapchain_manager.MinImageCount());
            camera.UpdateAspect(ComputeAspect(extent));
        }
        default: break;
        }
    }

    return 0;
}
