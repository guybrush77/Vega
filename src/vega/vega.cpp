#include "etna.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "utils/resource.hpp"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/matrix.hpp>

#include <algorithm>
#include <optional>
#include <spdlog/spdlog.h>
#include <unordered_map>
#include <vector>

DECLARE_VERTEX_ATTRIBUTE_TYPE(glm::vec3, etna::Format::R32G32B32Sfloat)

struct MVP {
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
};

struct Queues final {
    etna::Queue graphics;
    etna::Queue compute;
    etna::Queue transfer;
};

QueueFamilies GetQueueFamilies(etna::PhysicalDevice gpu)
{
    using namespace etna;

    auto properties = gpu.GetPhysicalDeviceQueueFamilyProperties();

    constexpr auto mask = QueueFlags::Graphics | QueueFlags::Compute | QueueFlags::Transfer;

    std::optional<QueueInfo> graphics;

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

    return { graphics.value(), compute.value(), transfer.value() };
}

int main()
{
#ifdef NDEBUG
    const bool enable_validation = false;
#else
    const bool enable_validation = true;
#endif

    using namespace etna;

    UniqueInstance instance;
    {
        std::vector<const char*> extensions;
        std::vector<const char*> layers;

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
    }

    auto [graphics, compute, transfer] = GetQueueFamilies(gpu);

    UniqueDevice device;
    {
        auto builder = Device::Builder();

        builder.AddQueue(graphics.family_index, 1);
        builder.AddQueue(compute.family_index, 1);
        builder.AddQueue(transfer.family_index, 1);

        builder.AddEnabledLayer(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        device = instance->CreateDevice(gpu, builder.state);
    }

    Queues queues;
    {
        queues.graphics = device->GetQueue(graphics.family_index);
        queues.compute  = device->GetQueue(compute.family_index);
        queues.transfer = device->GetQueue(transfer.family_index);
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

    // Create pipeline renderpass
    UniqueRenderPass renderpass;
    {
        auto builder = RenderPass::Builder();

        auto color_attachment = builder.AddAttachment(
            Format::R8G8B8A8Srgb,
            AttachmentLoadOp::Clear,
            AttachmentStoreOp::Store,
            ImageLayout::Undefined,
            ImageLayout::TransferSrcOptimal);

        auto depth_attachment = builder.AddAttachment(
            Format::D24UnormS8Uint,
            AttachmentLoadOp::Clear,
            AttachmentStoreOp::DontCare,
            ImageLayout::Undefined,
            ImageLayout::DepthStencilAttachmentOptimal);

        auto color_ref = builder.AddReference(color_attachment, ImageLayout::ColorAttachmentOptimal);
        auto depth_ref = builder.AddReference(depth_attachment, ImageLayout::DepthStencilAttachmentOptimal);

        auto subpass = builder.CreateSubpass();

        subpass.AddColorAttachment(color_ref);
        subpass.SetDepthStencilAttachment(depth_ref);

        builder.AddSubpass(subpass.state);

        renderpass = device->CreateRenderPass(builder.state);
    }

    // Create render target image
    UniqueImage2D image = device->CreateImage(
        Format::R8G8B8A8Srgb,
        Extent2D{ 640, 640 },
        ImageUsage::ColorAttachment | ImageUsage::TransferSrc,
        MemoryUsage::GpuOnly,
        ImageTiling::Optimal);

    UniqueImage2D depth = device->CreateImage(
        Format::D24UnormS8Uint,
        Extent2D{ 640, 640 },
        ImageUsage::DepthStencilAttachment,
        MemoryUsage::GpuOnly,
        ImageTiling::Optimal);

    // Create render target framebuffer
    auto image_view  = device->CreateImageView(*image, ImageAspect::Color);
    auto depth_view  = device->CreateImageView(*depth, ImageAspect::Depth);
    auto framebuffer = device->CreateFramebuffer(*renderpass, { *image_view, *depth_view }, image->Extent2D());

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
        auto [width, height]    = image->Extent2D();
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

    // Create and initialize descriptor sets
    auto descriptor_pool = device->CreateDescriptorPool(DescriptorType::UniformBuffer, 1);

    auto descriptor_set = descriptor_pool->AllocateDescriptorSet(*descriptor_set_layout);

    auto mvp_buffer = device->CreateBuffer(sizeof(MVP), BufferUsage::UniformBuffer, MemoryUsage::CpuToGpu);
    {
        auto eye    = glm::vec3(1, 1, 3);
        auto center = glm::vec3(0, 0, 0);
        auto up     = glm::vec3(0, -1, 0);

        MVP mvp{};

        mvp.view = glm::lookAtRH(eye, center, up);
        mvp.proj = glm::perspectiveRH(45.0f, 1.0f, 0.1f, 1000.0f);

        void* data = mvp_buffer->MapMemory();
        memcpy(data, &mvp, sizeof(mvp));
        mvp_buffer->UnmapMemory();

        WriteDescriptorSet write_descriptor(descriptor_set, Binding{ 0 }, DescriptorType::UniformBuffer);
        write_descriptor.AddBuffer(*mvp_buffer);
        device->UpdateDescriptorSet(write_descriptor);
    }

    // Render image
    {
        auto clear_color   = ClearColor::Transparent;
        auto depth_stencil = ClearDepthStencil::Default;

        auto cmd_pool   = device->CreateCommandPool(graphics.family_index, CommandPoolCreate::Transient);
        auto cmd_buffer = cmd_pool->AllocateCommandBuffer();

        cmd_buffer->Begin(CommandBufferUsage::OneTimeSubmit);
        cmd_buffer->BeginRenderPass(*framebuffer, { clear_color, depth_stencil }, SubpassContents::Inline);
        cmd_buffer->BindPipeline(PipelineBindPoint::Graphics, *pipeline);
        cmd_buffer->BindVertexBuffers(*vertex_buffer);
        cmd_buffer->BindIndexBuffer(*index_buffer, IndexType::Uint32);
        cmd_buffer->BindDescriptorSet(PipelineBindPoint::Graphics, *layout, descriptor_set);
        cmd_buffer->DrawIndexed(mesh.indices.count(), 1);
        cmd_buffer->EndRenderPass();
        cmd_buffer->End();

        queues.graphics.Submit(*cmd_buffer);
        device->WaitIdle();
    }

    // Transfer image to CPU
    UniqueImage2D dst_image;
    {
        dst_image = device->CreateImage(
            image->Format(),
            image->Extent2D(),
            ImageUsage::TransferDst,
            MemoryUsage::CpuOnly,
            ImageTiling::Linear);

        auto cmd_pool   = device->CreateCommandPool(transfer.family_index, CommandPoolCreate::Transient);
        auto cmd_buffer = cmd_pool->AllocateCommandBuffer();

        cmd_buffer->Begin(CommandBufferUsage::OneTimeSubmit);
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

        queues.transfer.Submit(*cmd_buffer);
        device->WaitIdle();
    }

    // Write image to file
    {
        auto [width, height] = dst_image->Extent2D();
        auto stride          = 4 * width;

        void* data = dst_image->MapMemory();
        stbi_write_png("out.png", width, height, 4, data, stride);
        dst_image->UnmapMemory();
    }

    return 0;
}
