#include "etna/etna.hpp"

#include "buffer_manager.hpp"
#include "camera.hpp"
#include "descriptor_manager.hpp"
#include "frame_manager.hpp"
#include "gui.hpp"
#include "render_context.hpp"
#include "scene.hpp"
#include "swapchain_manager.hpp"
#include "texture_loader.hpp"
#include "utils/misc.hpp"
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

enum class KhronosValidation { Disable, Enable };

struct Vertex final {
    constexpr Vertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& uv) noexcept
        : position(position), normal(normal), uv(uv)
    {}
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};

DECLARE_VERTEX_ATTRIBUTE_TYPE(glm::vec2, etna::Format::R32G32Sfloat)
DECLARE_VERTEX_ATTRIBUTE_TYPE(glm::vec3, etna::Format::R32G32B32Sfloat)

DECLARE_VERTEX_TYPE(Vertex, Position3f | Normal3f)

struct TinyIndex final {
    struct Hash final {
        size_t operator()(const tinyobj::index_t& index) const noexcept { return index.vertex_index; }
    };
    struct Equal final {
        bool operator()(const tinyobj::index_t& lhs, const tinyobj::index_t& rhs) const noexcept
        {
            return (lhs.vertex_index == rhs.vertex_index) && (lhs.normal_index == rhs.normal_index) &&
                   (lhs.texcoord_index == rhs.texcoord_index);
        }
    };
};

using IndexMap = std::unordered_map<tinyobj::index_t, size_t, TinyIndex::Hash, TinyIndex::Equal>;

struct GLFW {
    GLFW()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    }
    ~GLFW() { glfwTerminate(); }
} glfw;

struct MeshRecord final {
    AABB   aabb{};
    int    material_id{};
    size_t first_index{};
    size_t index_count{};
};

using MeshRecords = std::vector<MeshRecord>;

static MeshRecords GenerateMeshRecords(
    const tinyobj::attrib_t& attributes,
    const tinyobj::mesh_t&   mesh,
    IndexMap*                index_map,
    std::vector<Vertex>*     vertices,
    std::vector<uint32_t>*   indices)
{
    const auto& [positions, normals, texcoords, colors] = attributes;

    auto mesh_map = std::map<int, std::vector<uint32_t>>{};

    for (size_t i = 0; i < mesh.indices.size(); ++i) {
        const auto& index       = mesh.indices[i];
        const auto  material_id = mesh.material_ids[i / 3];
        const auto  pindex      = 3 * index.vertex_index;
        const auto  nindex      = 3 * index.normal_index;
        const auto  tindex      = 2 * index.texcoord_index;
        const auto  position    = glm::vec3(positions[pindex + 0], positions[pindex + 1], positions[pindex + 2]);
        const auto  normal      = glm::vec3(normals[nindex + 0], normals[nindex + 1], normals[nindex + 2]);
        const auto  texcoord    = glm::vec2(texcoords[tindex + 0], texcoords[tindex + 1]);

        auto new_index = vertices->size();

        if (auto [it, success] = index_map->try_emplace(index, vertices->size()); success) {
            vertices->emplace_back(position, normal, texcoord);
        } else {
            new_index = it->second;
        }

        auto& index_buffer = mesh_map[material_id];
        if (index_buffer.empty()) {
            index_buffer.reserve(mesh.indices.size());
        }
        index_buffer.push_back(utils::narrow_cast<uint32_t>(new_index));
    }

    auto mesh_records = MeshRecords{};

    for (auto& [material_id, index_buffer] : mesh_map) {
        auto record = MeshRecord{

            .aabb        = AABB{ { FLT_MAX, FLT_MAX, FLT_MAX }, { FLT_MIN, FLT_MIN, FLT_MIN } },
            .material_id = material_id,
            .first_index = indices->size(),
            .index_count = index_buffer.size()
        };

        for (uint32_t index : index_buffer) {
            auto position = (*vertices)[index].position;

            record.aabb.min = { std::min(record.aabb.min.x, position.x),
                                std::min(record.aabb.min.y, position.y),
                                std::min(record.aabb.min.z, position.z) };

            record.aabb.max = { std::max(record.aabb.max.x, position.x),
                                std::max(record.aabb.max.y, position.y),
                                std::max(record.aabb.max.z, position.z) };

            indices->push_back(index);
        }

        mesh_records.push_back(record);
    }

    return mesh_records;
}

static void GenerateNormals(tinyobj::attrib_t* attributes, std::vector<tinyobj::shape_t>* shapes)
{
    using namespace glm;
    using utils::narrow_cast;

    static constexpr float kMinDot = 0.999847695f;

    auto& [positions, out_normals, texcoords, colors] = *attributes;

    auto normals   = std::vector<tinyobj::real_t>{};
    auto index_map = std::unordered_map<int, int>{};

    index_map.reserve(positions.size());

    for (auto& [name, mesh, path] : *shapes) {
        for (size_t index = 0; index < mesh.indices.size(); index += 3) {
            auto indices = std::array{ &mesh.indices[index + 0], &mesh.indices[index + 1], &mesh.indices[index + 2] };
            auto pindex0 = 3 * narrow_cast<size_t>(indices[0]->vertex_index);
            auto pindex1 = 3 * narrow_cast<size_t>(indices[1]->vertex_index);
            auto pindex2 = 3 * narrow_cast<size_t>(indices[2]->vertex_index);
            auto p0      = vec3(positions[pindex0 + 0], positions[pindex0 + 1], positions[pindex0 + 2]);
            auto p1      = vec3(positions[pindex1 + 0], positions[pindex1 + 1], positions[pindex1 + 2]);
            auto p2      = vec3(positions[pindex2 + 0], positions[pindex2 + 1], positions[pindex2 + 2]);
            auto normal  = normalize(cross(p1 - p0, p2 - p0));

            for (size_t i = 0; i < indices.size(); ++i) {
                auto normal_index = narrow_cast<int>(normals.size());
                if (auto [it, emplaced] = index_map.try_emplace(indices[i]->vertex_index, normal_index); emplaced) {
                    normals.insert(normals.end(), { normal.x, normal.y, normal.z });
                } else {
                    auto current = vec3(normals[it->second + 0], normals[it->second + 1], normals[it->second + 2]);
                    if (dot(normal, current) >= kMinDot) {
                        normal_index = it->second;
                    } else {
                        normals.insert(normals.end(), { normal.x, normal.y, normal.z });
                    }
                }
                indices[i]->normal_index = normal_index / 3;
            }
        }
    }

    out_normals = std::move(normals);
}

static void GenerateTexcoords(tinyobj::attrib_t* attributes, std::vector<tinyobj::shape_t>* shapes)
{
    attributes->texcoords.clear();
    attributes->texcoords.push_back(0.0f);
    attributes->texcoords.push_back(0.0f);

    for (auto& [name, mesh, path] : *shapes) {
        for (auto& index : mesh.indices) {
            index.texcoord_index = 0;
        }
    }
}

static std::map<int, MaterialPtr> GenerateMaterials(
    ScenePtr                                scene,
    TextureLoader*                          texture_loader,
    const std::vector<tinyobj::material_t>& tiny_materials,
    const std::filesystem::path&            parent_dir)
{
    auto shader       = scene->CreateShader();
    auto material_map = std::map<int, MaterialPtr>{};
    {
        auto default_material = scene->CreateMaterial(shader);
        material_map[-1]      = default_material;

        auto material_index = 0;
        for (const auto& tiny_material : tiny_materials) {
            auto material = scene->CreateMaterial(shader);
            material->SetProperty("name", tiny_material.name);
            if (tiny_material.diffuse[0] > 0 || tiny_material.diffuse[1] > 0 || tiny_material.diffuse[2] > 0) {
                material->SetProperty("diffuse.color", Float3(tiny_material.diffuse));
            }
            if (false == tiny_material.diffuse_texname.empty()) {
                auto filepath = (parent_dir / tiny_material.diffuse_texname).string();
                texture_loader->LoadAsync(filepath);
                material->SetProperty("diffuse.texture", filepath);
            }

            auto index          = utils::narrow_cast<int>(material_index);
            material_map[index] = material;
            ++material_index;
        }
    }

    return material_map;
}

void LoadObj(ScenePtr scene, TextureLoader* texture_loader, std::filesystem::path filepath)
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
    auto error      = std::string{};

    spdlog::info("Parsing scene");

    bool success = tinyobj::LoadObj(
        &attributes,
        &shapes,
        &materials,
        &warning,
        &error,
        filepath.string().c_str(),
        parent_dir.string().c_str(),
        true,
        false);

    if (!success || !error.empty()) {
        spdlog::error("{}", error);
        utils::throw_runtime_error("Failed to load object file");
    }

    auto extension = filepath.extension().string();
    utils::to_lower(extension.data());

    if (extension != ".obj") {
        throw std::runtime_error("File is not an .obj file");
    }

    if (!warning.empty()) {
        spdlog::warn("{}", warning);
    }

    if (attributes.normals.empty()) {
        spdlog::info("Generating normals");
        GenerateNormals(&attributes, &shapes);
    }

    if (attributes.texcoords.empty()) {
        GenerateTexcoords(&attributes, &shapes);
    }

    auto material_map = GenerateMaterials(scene, texture_loader, materials, parent_dir);

    spdlog::info("Generating scene");

    auto root_node = scene->GetRootNode();
    auto file_node = root_node->AttachNode(scene->CreateGroupNode());

    file_node->SetProperty("name", filepath.filename().string());
    file_node->SetProperty("Path", filepath.string());

    auto mesh_map      = std::map<size_t, MeshRecords>{};
    auto vertex_buffer = VertexBufferPtr{};
    auto index_buffer  = IndexBufferPtr{};

    {
        auto index_count = size_t{ 0 };
        for (auto& shape : shapes) {
            index_count += shape.mesh.indices.size();
        }

        auto vertices = std::vector<Vertex>{};
        vertices.reserve(2 * attributes.vertices.size());

        auto indices = std::vector<uint32_t>{};
        indices.reserve(index_count);

        auto index_map = IndexMap{};
        index_map.reserve(2 * attributes.vertices.size());

        for (size_t shape_index = 0; shape_index < shapes.size(); ++shape_index) {
            auto records = GenerateMeshRecords(attributes, shapes[shape_index].mesh, &index_map, &vertices, &indices);
            mesh_map[shape_index] = std::move(records);
        }

        auto vertices_size = sizeof(vertices[0]) * vertices.size();
        vertex_buffer      = scene->CreateVertexBuffer(vertices.data(), vertices_size, std::align_val_t(32));

        auto indices_size = sizeof(indices[0]) * indices.size();
        index_buffer      = scene->CreateIndexBuffer(indices.data(), indices_size, std::align_val_t(32));
    }

    auto shape_num = 1;

    for (const auto& [shape_index, mesh_records] : mesh_map) {
        auto parent = file_node;
        auto name   = shapes[shape_index].name;
        if (name.empty()) {
            name = std::string("Mesh ") + std::to_string(shape_num++);
        }
        if (mesh_records.size() > 1) {
            parent = file_node->AttachNode(scene->CreateGroupNode());
            parent->SetProperty("name", name);
        }
        auto mesh_num = 1;
        for (const auto& [aabb, material_id, first, count] : mesh_records) {
            auto mesh     = scene->CreateMesh(aabb, vertex_buffer, index_buffer, first, count);
            auto material = material_map[material_id];
            auto instance = parent->AttachNode(scene->CreateInstanceNode(mesh, material));
            if (mesh_records.size() == 1) {
                instance->SetProperty("name", name);
            } else {
                auto suffix = std::string(" (") + std::to_string(mesh_num++) + (")");
                instance->SetProperty("name", name + suffix);
            }
        }
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

    auto it = std::ranges::find_first_of(available_formats, preffered_formats);

    return it == available_formats.end() ? available_formats.front() : *it;
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

struct GlfwWindowDeleter {
    void operator()(GLFWwindow* window) { glfwDestroyWindow(window); }
};

using GlfwWindow = std::unique_ptr<GLFWwindow, GlfwWindowDeleter>;

void GlfwErrorCallback(int, const char* description)
{
    spdlog::error("GLFW: {}", description);
}

GlfwWindow CreateGlfwWindow(const char* window_name)
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

auto CreateEtnaInstance(KhronosValidation khronos_validation) -> etna::UniqueInstance
{
    using namespace etna;

    utils::throw_runtime_error_if(false == glfwVulkanSupported(), "GLFW Vulkan not supported!");

    auto count           = 0U;
    auto glfw_extensions = glfwGetRequiredInstanceExtensions(&count);
    auto extensions      = std::vector<const char*>(glfw_extensions, glfw_extensions + count);
    auto layers          = std::vector<const char*>();

    if (khronos_validation == KhronosValidation::Enable) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        layers.push_back("VK_LAYER_KHRONOS_validation");
    }

    return CreateInstance(
        "Vega",
        Version{ 0, 1, 0 },
        extensions,
        layers,
        VulkanDebugCallback,
        DebugUtilsMessageSeverity::Warning | DebugUtilsMessageSeverity::Error,
        DebugUtilsMessageType::General | DebugUtilsMessageType::Performance | DebugUtilsMessageType::Validation);
}

auto GetEtnaGpu(etna::Instance instance)
{
    auto gpus                      = instance.EnumeratePhysicalDevices();
    auto gpu                       = gpus.front();
    auto queue_family_properties   = gpu.GetPhysicalDeviceQueueFamilyProperties();
    bool is_presentation_supported = false;

    for (uint32_t index = 0; index < queue_family_properties.size(); ++index) {
        is_presentation_supported |= (GLFW_TRUE == glfwGetPhysicalDevicePresentationSupport(instance, gpu, index));
    }

    utils::throw_runtime_error_if(!is_presentation_supported, "Failed to detect GPU queue that supports presentation");

    return gpu;
}

etna::UniqueSurfaceKHR CreateEtnaSurface(etna::Instance instance, GLFWwindow* glfw_window)
{
    auto vk_surface = VkSurfaceKHR{};

    auto success = (VK_SUCCESS == glfwCreateWindowSurface(instance, glfw_window, nullptr, &vk_surface));

    utils::throw_runtime_error_if(false == success, "Failed to create window surface");

    return etna::UniqueSurfaceKHR(etna::SurfaceKHR(instance, vk_surface));
}

etna::UniqueDevice GetEtnaDevice(etna::Instance instance, etna::PhysicalDevice gpu, const QueueFamilies& queue_families)
{
    auto queue_family_indices = RemoveDuplicates({

        queue_families.graphics.family_index,
        queue_families.compute.family_index,
        queue_families.transfer.family_index,
        queue_families.presentation.family_index });

    auto builder = etna::Device::Builder();

    for (auto queue_family_index : queue_family_indices) {
        builder.AddQueue(queue_family_index, 1);
    }

    builder.AddEnabledExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    return instance.CreateDevice(gpu, builder.state);
}

etna::Extent2D ComputeEtnaExtent(etna::PhysicalDevice gpu, GLFWwindow* glfw_window, etna::SurfaceKHR surface)
{
    int width{}, height{};
    glfwGetWindowSize(glfw_window, &width, &height);

    auto extent       = etna::Extent2D{ etna::narrow_cast<uint32_t>(width), etna::narrow_cast<uint32_t>(height) };
    auto capabilities = gpu.GetPhysicalDeviceSurfaceCapabilitiesKHR(surface);

    extent.width  = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    if (capabilities.currentExtent.width != UINT32_MAX) {
        extent = capabilities.currentExtent;
    }

    return extent;
}

class EventHandler {
  public:
    EventHandler(
        etna::Device   device,
        GLFWwindow*    glfw_window,
        RenderContext* render_context,
        Scene*         scene,
        Camera*        camera,
        BufferManager* buffer_manager,
        TextureLoader* texture_loader)
        : m_device(device), m_glfw_window(glfw_window), m_render_context(render_context), m_scene(scene),
          m_camera(camera), m_buffer_manager(buffer_manager), m_texture_loader(texture_loader)
    {}

    void ScheduleCloseWindow() noexcept
    {
        m_event = Event::CloseWindow;
        m_render_context->StopRenderLoop();
    }

    void ScheduleLoadFile(std::string filepath) noexcept
    {
        m_event                         = Event::LoadFile;
        m_load_file_parameters.filepath = std::move(filepath);
        m_render_context->StopRenderLoop();
    }

    void HandleEvent()
    {
        switch (m_event) {
        case Event::None: break;
        case Event::CloseWindow: CloseWindow(); break;
        case Event::LoadFile: LoadFile(); break;
        default: break;
        }

        m_event = Event::None;
    }

  private:
    enum class Event { None, CloseWindow, LoadFile };

    struct LoadFileParameters final {
        std::string filepath;
    } m_load_file_parameters;

    void CloseWindow() { glfwSetWindowShouldClose(m_glfw_window, GLFW_TRUE); }

    void LoadFile()
    {
        spdlog::info("Loading file {}", m_load_file_parameters.filepath);

        auto start = std::chrono::system_clock::now();

        LoadObj(m_scene, m_texture_loader, m_load_file_parameters.filepath);

        auto draw_list = m_scene->ComputeDrawList();
        for (const DrawRecord& draw_record : draw_list) {
            m_buffer_manager->CreateBuffer(draw_record.mesh->GetVertexBuffer(), etna::BufferUsage::VertexBuffer);
            m_buffer_manager->CreateBuffer(draw_record.mesh->GetIndexBuffer(), etna::BufferUsage::IndexBuffer);
        }

        spdlog::info("Uploading data");

        m_buffer_manager->UploadAsync();
        m_texture_loader->UploadAsync();

        m_device.WaitIdle();

        m_buffer_manager->CleanAfterUpload();
        m_texture_loader->CleanAfterUpload();

        auto elapsed = duration_cast<std::chrono::duration<double>>(std::chrono::system_clock::now() - start).count();

        spdlog::info("File loaded. Elapsed time: {} seconds.", elapsed);

        auto aabb = m_scene->ComputeAxisAlignedBoundingBox();

        int width{}, height{};
        glfwGetWindowSize(m_glfw_window, &width, &height);

        auto aspect = static_cast<float>(width) / static_cast<float>(height);

        *m_camera = Camera::Create(
            Orientation::RightHanded,
            Forward{ Axis::PositiveY },
            Up{ Axis::PositiveZ },
            ObjectView::Front,
            aabb,
            45_deg,
            aspect);
    }

    etna::Device   m_device;
    GLFWwindow*    m_glfw_window;
    RenderContext* m_render_context;
    Scene*         m_scene;
    Camera*        m_camera;
    BufferManager* m_buffer_manager;
    TextureLoader* m_texture_loader;
    Event          m_event = Event::None;
};

int main()
{
#ifdef NDEBUG
    const KhronosValidation khronos_validation = KhronosValidation::Disable;
#else
    const KhronosValidation khronos_validation = KhronosValidation::Enable;
#endif

    using namespace etna;

    auto instance       = CreateEtnaInstance(khronos_validation);
    auto gpu            = GetEtnaGpu(instance.get());
    auto gpu_properties = gpu.GetPhysicalDeviceProperties();

    spdlog::info("GPU Info: {}, {}", gpu_properties.deviceName, to_string(gpu_properties.deviceType));
    spdlog::info("GLFW Version: {}", glfwGetVersionString());

    glfwSetErrorCallback(GlfwErrorCallback);

    auto glfw_window              = CreateGlfwWindow("Vega Viewer");
    auto surface                  = CreateEtnaSurface(instance.get(), glfw_window.get());
    auto extent                   = ComputeEtnaExtent(gpu, glfw_window.get(), surface.get());
    auto aspect                   = ComputeAspect(extent);
    auto preffered_surface_format = SurfaceFormatKHR{ Format::B8G8R8A8Srgb, ColorSpaceKHR::SrgbNonlinear };
    auto surface_format           = FindOptimalSurfaceFormatKHR(gpu, surface.get(), { preffered_surface_format });
    auto depth_format             = FindSupportedFormat(
        gpu,
        { Format::D24UnormS8Uint, Format::D32SfloatS8Uint, Format::D16Unorm },
        ImageTiling::Optimal,
        FormatFeature::DepthStencilAttachment);

    spdlog::info("Surface Format: {}, {}", to_string(surface_format.format), to_string(surface_format.colorSpace));

    auto queue_families = GetQueueFamilyInfo(gpu, surface.get());
    auto device         = GetEtnaDevice(instance.get(), gpu, queue_families);
    auto queues         = Queues{};
    {
        queues.graphics     = device->GetQueue(queue_families.graphics.family_index);
        queues.compute      = device->GetQueue(queue_families.compute.family_index);
        queues.transfer     = device->GetQueue(queue_families.transfer.family_index);
        queues.presentation = device->GetQueue(queue_families.presentation.family_index);
    }

    // Create pipeline renderpass
    auto renderpass = UniqueRenderPass();
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
    auto gui_renderpass = UniqueRenderPass();
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

    // Create transforms set layout
    auto transforms_set_layout = UniqueDescriptorSetLayout();
    {
        auto builder = DescriptorSetLayout::Builder();

        builder
            .AddDescriptorSetLayoutBinding(Binding{ 0 }, DescriptorType::UniformBufferDynamic, 1, ShaderStage::Vertex);
        builder.AddDescriptorSetLayoutBinding(Binding{ 1 }, DescriptorType::UniformBuffer, 1, ShaderStage::Vertex);
        builder.AddDescriptorSetLayoutBinding(Binding{ 2 }, DescriptorType::UniformBuffer, 1, ShaderStage::Fragment);

        transforms_set_layout = device->CreateDescriptorSetLayout(builder.state);
    }

    // Create textures set layout
    auto textures_set_layout = UniqueDescriptorSetLayout();
    {
        auto builder = DescriptorSetLayout::Builder();

        builder.AddDescriptorSetLayoutBinding(
            Binding{ 10 },
            DescriptorType::CombinedImageSampler,
            1,
            ShaderStage::Fragment);

        textures_set_layout = device->CreateDescriptorSetLayout(builder.state);
    }

    // Create pipeline layout
    auto pipeline_layout = UniquePipelineLayout();
    {
        auto builder = PipelineLayout::Builder();
        builder.AddDescriptorSetLayout(*transforms_set_layout);
        builder.AddDescriptorSetLayout(*textures_set_layout);
        pipeline_layout = device->CreatePipelineLayout(builder.state);
    }

    // Create pipeline
    auto pipeline = UniquePipeline();
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
        builder.AddVertexInputBindingDescription(Binding{ 0 }, sizeof(Vertex));
        builder.AddVertexInputAttributeDescription(
            Location{ 0 },
            Binding{ 0 },
            formatof(Vertex, position),
            offsetof(Vertex, position));
        builder.AddVertexInputAttributeDescription(
            Location{ 1 },
            Binding{ 0 },
            formatof(Vertex, normal),
            offsetof(Vertex, normal));
        builder.AddVertexInputAttributeDescription(
            Location{ 2 },
            Binding{ 0 },
            formatof(Vertex, uv),
            offsetof(Vertex, uv));
        builder.AddViewport(viewport);
        builder.AddScissor(scissor);
        builder.AddDynamicStates({ DynamicState::Viewport, DynamicState::Scissor });
        builder.SetDepthState(DepthTest::Enable, DepthWrite::Enable, CompareOp::Less);
        builder.AddColorBlendAttachmentState();

        pipeline = device->CreateGraphicsPipeline(builder.state);
    }

    auto texture_loader = TextureLoader(*device, queues.graphics);
    auto buffer_manager = BufferManager(*device, queues.transfer);

    uint32_t image_count = 3;
    uint32_t frame_count = 2;

    auto descriptor_manager =
        DescriptorManager(*device, frame_count, *transforms_set_layout, *textures_set_layout, gpu_properties.limits);

    auto render_context = RenderContext();

    auto scene = Scene();

    auto aabb = AABB();

    auto camera = Camera::Create(
        Orientation::RightHanded,
        Forward{ Axis::PositiveY },
        Up{ Axis::PositiveZ },
        ObjectView::Front,
        aabb,
        45_deg,
        aspect);

    auto lights = Lights();
    {
        lights.KeyRef().MultiplierRef() = 0.7f;
        lights.KeyRef().ElevationRef()  = ToRadians(45_deg).value;
        lights.KeyRef().AzimuthRef()    = ToRadians(-45_deg).value;

        lights.FillRef().MultiplierRef() = 0.05f;
        lights.FillRef().ElevationRef()  = ToRadians(5_deg).value;
        lights.FillRef().AzimuthRef()    = ToRadians(25_deg).value;
    }

    auto event_handler = EventHandler(
        device.get(),
        glfw_window.get(),
        &render_context,
        &scene,
        &camera,
        &buffer_manager,
        &texture_loader);

    auto parameters = Gui::Parameters{

        .instance       = *instance,
        .gpu            = gpu,
        .device         = *device,
        .graphics_queue = queues.graphics,
        .renderpass     = *gui_renderpass,
        .extent         = extent
    };

    auto callbacks = Gui::Callbacks{

        .OnWindowClose = [&event_handler]() { event_handler.ScheduleCloseWindow(); },
        .OnFileOpen = [&event_handler](std::string filepath) { event_handler.ScheduleLoadFile(std::move(filepath)); }
    };

    auto gui = Gui(parameters, callbacks, glfw_window.get(), image_count, image_count, &camera, &scene, &lights);

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

        auto frame_manager = FrameManager(*device, queue_families.graphics.family_index, frame_count);

        render_context = RenderContext(
            *device,
            queues.graphics,
            *pipeline,
            *pipeline_layout,
            glfw_window.get(),
            &swapchain_manager,
            &frame_manager,
            &descriptor_manager,
            &gui,
            &camera,
            &lights,
            &buffer_manager,
            &texture_loader,
            &scene);

        auto status = render_context.StartRenderLoop();

        device->WaitIdle();

        switch (status) {
        case RenderContext::Status::WindowClosed: {
            running = false;
            break;
        }
        case RenderContext::Status::SwapchainOutOfDate: {
            int width  = 0;
            int height = 0;
            while (width == 0 && height == 0) {
                glfwGetWindowSize(glfw_window.get(), &width, &height);
                if (width == 0 && height == 0) {
                    glfwWaitEvents();
                }
            }
            extent.width  = narrow_cast<uint32_t>(width);
            extent.height = narrow_cast<uint32_t>(height);
            gui.UpdateViewport(extent, swapchain_manager.MinImageCount());
            camera.UpdateAspect(ComputeAspect(extent));
            break;
        }
        case RenderContext::Status::GuiEvent: {
            event_handler.HandleEvent();
            break;
        }
        default: break;
        }
    }

    return 0;
}
