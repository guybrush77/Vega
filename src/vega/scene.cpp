#include "scene.hpp"

#include "utils/cast.hpp"

#include <atomic>
#include <ranges>
#include <tuple>

static void to_json(json& json, const Float3& vec)
{
    json = { vec.x, vec.y, vec.z };
}

static void to_json(json& json, ID id)
{
    json = id.value;
}

struct ValueToJson final {
    void operator()(std::monostate) {}
    void operator()(ObjectPtr value) { j[key] = value->GetID(); }
    void operator()(auto& value) { j[key] = value; }

    json&              j;
    const std::string& key;
};

static void to_json(json& json, const UniqueNode& node)
{
    json = node->ToJson();
}

static void to_json(json& json, const UniqueMaterial& material)
{
    json = material->ToJson();
}

static void to_json(json& json, const AABB& aabb)
{
    json["aabb.min"] = aabb.min;
    json["aabb.max"] = aabb.max;
}

static void to_json(json& json, ShaderPtr shader)
{
    json = shader->ToJson();
}

static void to_json(json& json, InstanceNodePtr instance_node)
{
    json = GetID(instance_node);
}

static void to_json(json& json, MeshPtr mesh)
{
    json = mesh->ToJson();
}

static void to_json(json& json, const MeshVertices& vertices)
{
    json["vertex.attributes"] = vertices.GetVertexAttributes();
    json["vertex.size"]       = vertices.GetVertexSize();
    json["vertex.count"]      = vertices.GetCount();
    json["vertices.size"]     = vertices.GetSize();
}

static void to_json(json& json, const MeshIndices& indices)
{
    json["index.class"]  = indices.GetIndexType();
    json["index.size"]   = indices.GetIndexSize();
    json["index.count"]  = indices.GetCount();
    json["indices.size"] = indices.GetSize();
}

struct Properties final {
    const PropertyStore* ptr = nullptr;
};

static void to_json(json& json, const Properties& properties)
{
    for (const auto& [key, value] : *properties.ptr) {
        std::visit(ValueToJson{ json, key }, value);
    }
}

struct RotateValues final {
    const Float3 axis;
    const float  angle;
};

static void to_json(json& json, const RotateValues& values)
{
    json["rotate.axis"]  = values.axis;
    json["rotate.angle"] = values.angle;
}

struct TranslateValues final {
    const Float3 distance;
};

static void to_json(json& json, const TranslateValues& values)
{
    json["translate"] = values.distance;
}

struct ScaleValues final {
    const float factor;
};

static void to_json(json& json, const ScaleValues& values)
{
    json["scale"] = values.factor;
}

struct MeshNodeRefs final {
    const ID material;
    const ID mesh;
};

static void to_json(json& json, const MeshNodeRefs& values)
{
    json["material"] = values.material;
    json["mesh"]     = values.mesh;
}

struct MeshValues final {
    const AABB*         aabb;
    const MeshVertices* vertices;
    const MeshIndices*  indices;
};

static void to_json(json& json, const MeshValues& values)
{
    json["aabb"]     = *values.aabb;
    json["vertices"] = *values.vertices;
    json["indices"]  = *values.indices;
}

static ID GetUniqueID() noexcept
{
    static std::atomic_int id = 0;
    id++;
    return ID(id);
}

struct ObjectAccess final {
    template <typename T, typename... Args>
    static auto MakeUnique(Args&&... args)
    {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }

    template <typename T>
    static std::string GenerateFieldMetadata(const T& object, size_t index)
    {
        return std::string(object.kFieldWritable[index] ? "w:" : "r:") + std::string(object.kFieldNames[index]);
    }

    template <typename T, typename Fields>
    static PropertyValue GetProperty(std::string_view name, const T& object, const Fields& fields)
    {
        if (name == "_class") {
            return std::string(object.kClassName);
        }
        if (name == "_name") {
            return std::string(object.kDefaultName);
        }
        if (name == "_id") {
            return object.GetID().value;
        }
        if constexpr (std::tuple_size<Fields>::value > 0) {
            if (name == "_field.1.meta") {
                return GenerateFieldMetadata(object, 0);
            } else if (name == "field.1") {
                return std::get<0>(fields);
            }
        }
        if constexpr (std::tuple_size<Fields>::value > 1) {
            if (name == "_field.2.meta") {
                return GenerateFieldMetadata(object, 1);
            } else if (name == "field.2") {
                return std::get<1>(fields);
            }
        }
        if constexpr (std::tuple_size<Fields>::value > 2) {
            if (name == "_field.3.meta") {
                return GenerateFieldMetadata(object, 2);
            } else if (name == "field.3") {
                return std::get<2>(fields);
            }
        }
        if constexpr (std::tuple_size<Fields>::value > 3) {
            if (name == "_field.4.meta") {
                return GenerateFieldMetadata(object, 3);
            } else if (name == "field.4") {
                return std::get<3>(fields);
            }
        }
        if (auto it = object.m_properties.find(std::string(name)); it != object.m_properties.end()) {
            return it->second;
        }
        return {};
    }

    template <typename T, typename Fields>
    static PropertyValue
    GetProperty(std::string_view primary, std::string_view alternative, const T& object, const Fields& fields)
    {
        if (auto value = ObjectAccess::GetProperty(primary, object, fields); value.index() != 0) {
            return value;
        }
        return ObjectAccess::GetProperty(alternative, object, fields);
    }

    template <typename T, typename Args>
    static std::vector<Property> GetProperties(const T* obj, const Args& args)
    {
        auto make_property = [](auto& kv) { return Property{ kv.first, kv.second }; };
        auto view          = std::views::transform(obj->m_properties, make_property);
        auto properties    = std::vector<Property>(view.begin(), view.end());

        properties.push_back({ "_class", std::string(obj->kClassName) });
        properties.push_back({ "_name", std::string(obj->kDefaultName) });
        properties.push_back({ "_id", obj->GetID().value });

        if constexpr (std::tuple_size<Args>::value > 0) {
            properties.push_back({ "field.1", std::get<0>(args) });
        }
        if constexpr (std::tuple_size<Args>::value > 1) {
            properties.push_back({ "field.2", std::get<1>(args) });
        }
        if constexpr (std::tuple_size<Args>::value > 2) {
            properties.push_back({ "field.3", std::get<2>(args) });
        }
        if constexpr (std::tuple_size<Args>::value > 3) {
            properties.push_back({ "field.4", std::get<3>(args) });
        }
        return properties;
    }

    template <typename T, typename Args>
    static bool SetProperty(T& object, std::string_view name, const PropertyValue& value, Args args)
    {
        utils::throw_runtime_error_if(name.empty(), "Cannot set property: property name is missing");
        utils::throw_runtime_error_if(name.starts_with('_'), "Cannot set property: builtin property");

        if (name.starts_with("field.")) {
            name.remove_prefix(strlen("field."));
            if constexpr (std::tuple_size<Args>::value > 0) {
                if (name == "1") {
                    using Arg          = std::remove_pointer_t<std::tuple_element_t<0, Args>>;
                    *std::get<0>(args) = std::get<Arg>(value);
                    return true;
                }
            }
            if constexpr (std::tuple_size<Args>::value > 1) {
                if (name == "2") {
                    using Arg          = std::remove_pointer_t<std::tuple_element_t<1, Args>>;
                    *std::get<1>(args) = std::get<Arg>(value);
                    return true;
                }
            }
            if constexpr (std::tuple_size<Args>::value > 2) {
                if (name == "3") {
                    using Arg          = std::remove_pointer_t<std::tuple_element_t<2, Args>>;
                    *std::get<2>(args) = std::get<Arg>(value);
                    return true;
                }
            }
            if constexpr (std::tuple_size<Args>::value > 3) {
                if (name == "4") {
                    using Arg          = std::remove_pointer_t<std::tuple_element_t<3, Args>>;
                    *std::get<3>(args) = std::get<Arg>(value);
                    return true;
                }
            }
        }

        auto [it, inserted] = object.m_properties.insert_or_assign(std::string(name), value);

        return inserted;
    }

    template <size_t Fields>
    static bool RemoveProperty(Object& object, std::string_view name)
    {
        utils::throw_runtime_error_if(name.empty(), "Cannot remove property: property name is missing");
        utils::throw_runtime_error_if(name.starts_with('_'), "Cannot remove property: builtin property");

        if (name.starts_with("field.")) {
            name.remove_prefix(strlen("field."));
            if constexpr (Fields > 0) {
                utils::throw_runtime_error_if(name == "1", "Cannot remove property: builtin property");
            }
            if constexpr (Fields > 1) {
                utils::throw_runtime_error_if(name == "2", "Cannot remove property: builtin property");
            }
            if constexpr (Fields > 2) {
                utils::throw_runtime_error_if(name == "3", "Cannot remove property: builtin property");
            }
            if constexpr (Fields > 3) {
                utils::throw_runtime_error_if(name == "4", "Cannot remove property: builtin property");
            }
        }

        return object.m_properties.erase(std::string(name));
    }

    template <typename T>
    static auto HasChildren(const T* parent)
    {
        return !parent->m_children.empty();
    }

    template <typename T>
    static auto GetChildren(const T* parent)
    {
        Nodes nodes;
        nodes.reserve(parent->m_children.size());
        for (const auto& node : parent->m_children) {
            nodes.push_back(node.get());
        }
        return nodes;
    }

    static void AddInstancePtr(InstanceNodePtr instance_node, MaterialPtr material)
    {
        assert(instance_node && material);
        material->AddInstanceNodePtr(instance_node);
    }

    static NodePtr AttachNode(InnerNode* parent, UniqueNode child)
    {
        assert(parent && child);
        auto child_ref  = child.get();
        child->m_parent = parent;
        parent->m_children.push_back(std::move(child));
        return child_ref;
    }

    static UniqueNode DetachNode(NodePtr node)
    {
        assert(node);

        auto parent = static_cast<InnerNode*>(node->m_parent);

        utils::throw_runtime_error_if(parent == nullptr, "Cannot detach node: node has no parent");

        auto it = std::ranges::find_if(parent->m_children, [node](auto& child) { return child.get() == node; });

        utils::throw_runtime_error_if(it == parent->m_children.end(), "Cannot detach node: invariant violated");

        auto unique_node = std::move(*it);

        unique_node->m_parent = nullptr;

        parent->m_children.erase(it);

        return unique_node;
    }

    static bool IsAncestor(const Node* ancestor, const Node* node)
    {
        assert(node && ancestor);
        do {
            node = node->m_parent;
            if (node == ancestor) {
                return true;
            }
        } while (node);
        return false;
    }

    static void ApplyTransform(NodePtr node, const glm::mat4& matrix) { node->ApplyTransform(matrix); }

    template <typename T>
    static void ThisToJson(const T* object, json& json)
    {
        json["object.class"]      = object->kClassName;
        json["object.id"]         = GetID(object);
        json["object.properties"] = Properties{ &object->m_properties };
    }

    template <typename T>
    static void ChildrenToJson(const std::vector<T>& children, json& json)
    {
        json["owns"] = children;
    }
};

json Mesh::ToJson() const
{
    json json;

    ObjectAccess::ThisToJson(this, json);
    json["object.values"] = MeshValues{ &m_aabb, &m_vertices, &m_indices };

    return json;
}

PropertyValue Mesh::GetProperty(std::string_view name) const
{
    return ObjectAccess::GetProperty(
        name,
        *this,
        std::make_tuple(m_aabb.min, m_aabb.max, m_vertices.GetCount(), m_indices.GetCount() / 3));
}

PropertyValue Mesh::GetProperty(std::string_view primary, std::string_view alternative) const
{
    return ObjectAccess::GetProperty(
        primary,
        alternative,
        *this,
        std::make_tuple(m_aabb.min, m_aabb.max, m_vertices.GetCount(), m_indices.GetCount() / 3));
}

std::vector<Property> Mesh::GetProperties() const
{
    return ObjectAccess::GetProperties(
        this,
        std::make_tuple(m_aabb.min, m_aabb.max, m_vertices.GetCount(), m_indices.GetCount() / 3));
}

bool Mesh::SetProperty(std::string_view /*name*/, const PropertyValue& /*value*/)
{
    // TODO
    return false;
}

bool Mesh::RemoveProperty(std::string_view name)
{
    return ObjectAccess::RemoveProperty<kFieldNames.size()>(*this, name);
}

NodePtr InnerNode::AttachNode(UniqueNode node)
{
    return ObjectAccess::AttachNode(this, std::move(node));
}

UniqueNode InnerNode::DetachNode()
{
    return ObjectAccess::DetachNode(this);
}

bool InnerNode::IsAncestor(NodePtr node) const
{
    return ObjectAccess::IsAncestor(this, node);
}

bool InnerNode::HasChildren() const
{
    return ObjectAccess::HasChildren(this);
}

Nodes InnerNode::GetChildren() const
{
    return ObjectAccess::GetChildren(this);
}

Scene::Scene()
{
    m_root = ObjectAccess::MakeUnique<RootNode>(nullptr, GetUniqueID());
}

DrawList Scene::ComputeDrawList() const
{
    using namespace std::ranges;

    ObjectAccess::ApplyTransform(m_root.get(), glm::identity<glm::mat4>());

    auto draw_list = DrawList{};

    for (const auto& shader : m_shaders) {
        for (auto material : shader->GetMaterials()) {
            for (auto instance : material->GetInstanceNodes()) {
                draw_list.push_back({ instance->GetMeshPtr(), instance->GetTransformPtr() });
            }
        }
    }

    return draw_list;
}

AABB Scene::ComputeAxisAlignedBoundingBox() const
{
    using namespace std::ranges;

    if (m_root->GetChildren().empty()) {
        return AABB{ { -1, -1, -1 }, { 1, 1, 1 } };
    }

    ObjectAccess::ApplyTransform(m_root.get(), glm::identity<glm::mat4>());

    auto out = AABB{ { FLT_MAX, FLT_MAX, FLT_MAX }, { FLT_MIN, FLT_MIN, FLT_MIN } };

    for (const auto& shader : m_shaders) {
        for (auto material : shader->GetMaterials()) {
            for (auto instance : material->GetInstanceNodes()) {
                if (auto* mesh = instance->GetMeshPtr()) {
                    auto& model = *instance->GetTransformPtr();
                    auto& aabb  = mesh->GetBoundingBox();
                    auto  vec_a = model * glm::vec4(aabb.min.x, aabb.min.y, aabb.min.z, 1);
                    auto  vec_b = model * glm::vec4(aabb.max.x, aabb.max.y, aabb.max.z, 1);
                    out.min.x   = std::min({ out.min.x, vec_a.x, vec_b.x });
                    out.min.y   = std::min({ out.min.y, vec_a.y, vec_b.y });
                    out.min.z   = std::min({ out.min.z, vec_a.z, vec_b.z });
                    out.max.x   = std::max({ out.max.x, vec_a.x, vec_b.x });
                    out.max.y   = std::max({ out.max.y, vec_a.y, vec_b.y });
                    out.max.z   = std::max({ out.max.z, vec_a.z, vec_b.z });
                }
            }
        }
    }

    return out;
}

PropertyValue RootNode::GetProperty(std::string_view name) const
{
    return ObjectAccess::GetProperty(name, *this, std::make_tuple());
}

PropertyValue RootNode::GetProperty(std::string_view primary, std::string_view alternative) const
{
    return ObjectAccess::GetProperty(primary, alternative, *this, std::make_tuple());
}

std::vector<Property> RootNode::GetProperties() const
{
    return ObjectAccess::GetProperties(this, std::make_tuple());
}

bool RootNode::SetProperty(std::string_view name, const PropertyValue& value)
{
    return ObjectAccess::SetProperty(*this, name, value, std::make_tuple());
}

bool RootNode::RemoveProperty(std::string_view name)
{
    return ObjectAccess::RemoveProperty<kFieldNames.size()>(*this, name);
}

UniqueNode RootNode::DetachNode()
{
    utils::throw_runtime_error("Cannot detach node: root cannot be detached");
    return nullptr;
}

json RootNode::ToJson() const
{
    json json;

    ObjectAccess::ThisToJson(this, json);
    ObjectAccess::ChildrenToJson(m_children, json);

    return json;
}

void RootNode::ApplyTransform(const glm::mat4& matrix) noexcept
{
    for (auto& node : m_children) {
        ObjectAccess::ApplyTransform(static_cast<NodePtr>(node.get()), matrix);
    }
}

PropertyValue GroupNode::GetProperty(std::string_view name) const
{
    return ObjectAccess::GetProperty(name, *this, std::make_tuple());
}

PropertyValue GroupNode::GetProperty(std::string_view primary, std::string_view alternative) const
{
    return ObjectAccess::GetProperty(primary, alternative, *this, std::make_tuple());
}

std::vector<Property> GroupNode::GetProperties() const
{
    return ObjectAccess::GetProperties(this, std::make_tuple());
}

bool GroupNode::SetProperty(std::string_view name, const PropertyValue& value)
{
    return ObjectAccess::SetProperty(*this, name, value, std::make_tuple());
}

bool GroupNode::RemoveProperty(std::string_view name)
{
    return ObjectAccess::RemoveProperty<kFieldNames.size()>(*this, name);
}

json GroupNode::ToJson() const
{
    json json;

    ObjectAccess::ThisToJson(this, json);
    ObjectAccess::ChildrenToJson(m_children, json);

    return json;
}

void GroupNode::ApplyTransform(const glm::mat4& matrix) noexcept
{
    for (auto& node : m_children) {
        ObjectAccess::ApplyTransform(static_cast<NodePtr>(node.get()), matrix);
    }
}

bool InstanceNode::IsAncestor(NodePtr node) const
{
    return ObjectAccess::IsAncestor(this, node);
}

json InstanceNode::ToJson() const
{
    json json;

    ObjectAccess::ThisToJson(this, json);

    json["object.refs"] = MeshNodeRefs{ m_material->GetID(), m_mesh->GetID() };

    return json;
}

InstanceNode::~InstanceNode() noexcept
{
    if (m_material) {
        m_material->RemoveInstance(this);
    }
}

PropertyValue InstanceNode::GetProperty(std::string_view name) const
{
    return ObjectAccess::GetProperty(name, *this, std::make_tuple(m_mesh, m_material));
}

PropertyValue InstanceNode::GetProperty(std::string_view primary, std::string_view alternative) const
{
    return ObjectAccess::GetProperty(primary, alternative, *this, std::make_tuple(m_mesh, m_material));
}

std::vector<Property> InstanceNode::GetProperties() const
{
    return ObjectAccess::GetProperties(this, std::make_tuple(m_mesh, m_material));
}

bool InstanceNode::SetProperty(std::string_view name, const PropertyValue& value)
{
    auto mesh     = static_cast<ObjectPtr>(m_mesh);
    auto material = static_cast<ObjectPtr>(m_material);
    return ObjectAccess::SetProperty(*this, name, value, std::make_tuple(&mesh, &material));
}

bool InstanceNode::RemoveProperty(std::string_view name)
{
    return ObjectAccess::RemoveProperty<kFieldNames.size()>(*this, name);
}

NodePtr InstanceNode::AttachNode(UniqueNode /*node*/)
{
    utils::throw_runtime_error("Cannot attach node: cannot attach to leaf node");
    return nullptr;
}

UniqueNode InstanceNode::DetachNode()
{
    return ObjectAccess::DetachNode(this);
}

void InstanceNode::ApplyTransform(const glm::mat4& matrix) noexcept
{
    m_transform = matrix;
}

InstanceNode::InstanceNode(NodePtr parent, ID id, MeshPtr mesh, MaterialPtr material) noexcept
    : Node(parent, id), m_mesh(mesh), m_material(material)
{
    ObjectAccess::AddInstancePtr(this, material);
}

PropertyValue TranslateNode::GetProperty(std::string_view name) const
{
    return ObjectAccess::GetProperty(name, *this, std::make_tuple(m_distance));
}

PropertyValue TranslateNode::GetProperty(std::string_view primary, std::string_view alternative) const
{
    return ObjectAccess::GetProperty(primary, alternative, *this, std::make_tuple(m_distance));
}

std::vector<Property> TranslateNode::GetProperties() const
{
    return ObjectAccess::GetProperties(this, std::make_tuple(m_distance));
}

bool TranslateNode::SetProperty(std::string_view name, const PropertyValue& value)
{
    return ObjectAccess::SetProperty(*this, name, value, std::make_tuple(&m_distance));
}

bool TranslateNode::RemoveProperty(std::string_view name)
{
    return ObjectAccess::RemoveProperty<kFieldNames.size()>(*this, name);
}

json TranslateNode::ToJson() const
{
    json json;

    ObjectAccess::ThisToJson(this, json);
    ObjectAccess::ChildrenToJson(m_children, json);

    json["object.values"] = TranslateValues{ m_distance };

    return json;
}

void TranslateNode::ApplyTransform(const glm::mat4& matrix) noexcept
{
    for (auto& node : m_children) {
        auto node_ptr = static_cast<NodePtr>(node.get());
        auto distance = glm::vec3(m_distance.x, m_distance.y, m_distance.z);
        ObjectAccess::ApplyTransform(node_ptr, matrix * glm::translate(distance));
    }
}

PropertyValue RotateNode::GetProperty(std::string_view name) const
{
    return ObjectAccess::GetProperty(name, *this, std::make_tuple(m_axis, m_angle.value));
}

PropertyValue RotateNode::GetProperty(std::string_view primary, std::string_view alternative) const
{
    return ObjectAccess::GetProperty(primary, alternative, *this, std::make_tuple(m_axis, m_angle.value));
}

std::vector<Property> RotateNode::GetProperties() const
{
    return ObjectAccess::GetProperties(this, std::make_tuple(m_axis, m_angle.value));
}

bool RotateNode::SetProperty(std::string_view name, const PropertyValue& value)
{
    return ObjectAccess::SetProperty(*this, name, value, std::make_tuple(&m_axis, &m_angle.value));
}

bool RotateNode::RemoveProperty(std::string_view name)
{
    return ObjectAccess::RemoveProperty<kFieldNames.size()>(*this, name);
}

json RotateNode::ToJson() const
{
    json json;

    ObjectAccess::ThisToJson(this, json);
    ObjectAccess::ChildrenToJson(m_children, json);

    json["object.values"] = RotateValues{ m_axis, m_angle.value };

    return json;
}

void RotateNode::ApplyTransform(const glm::mat4& matrix) noexcept
{
    for (auto& node : m_children) {
        auto node_ptr = static_cast<NodePtr>(node.get());
        auto axis     = glm::vec3(m_axis.x, m_axis.y, m_axis.z);
        ObjectAccess::ApplyTransform(node_ptr, matrix * glm::rotate(m_angle.value, axis));
    }
}

PropertyValue ScaleNode::GetProperty(std::string_view name) const
{
    return ObjectAccess::GetProperty(name, *this, std::make_tuple(m_factor));
}

PropertyValue ScaleNode::GetProperty(std::string_view primary, std::string_view alternative) const
{
    return ObjectAccess::GetProperty(primary, alternative, *this, std::make_tuple(m_factor));
}

std::vector<Property> ScaleNode::GetProperties() const
{
    return ObjectAccess::GetProperties(this, std::make_tuple(m_factor));
}

bool ScaleNode::SetProperty(std::string_view name, const PropertyValue& value)
{
    return ObjectAccess::SetProperty(*this, name, value, std::make_tuple(&m_factor));
}

bool ScaleNode::RemoveProperty(std::string_view name)
{
    return ObjectAccess::RemoveProperty<kFieldNames.size()>(*this, name);
}

json ScaleNode::ToJson() const
{
    json json;

    ObjectAccess::ThisToJson(this, json);
    ObjectAccess::ChildrenToJson(m_children, json);

    json["object.values"] = ScaleValues{ m_factor };

    return json;
}

void ScaleNode::ApplyTransform(const glm::mat4& matrix) noexcept
{
    for (auto& node : m_children) {
        auto node_ptr = static_cast<NodePtr>(node.get());
        ObjectAccess::ApplyTransform(node_ptr, matrix * glm::scale(glm::vec3{ m_factor, m_factor, m_factor }));
    }
}

RootNodePtr Scene::GetRootNode() noexcept
{
    return static_cast<RootNodePtr>(m_root.get());
}

UniqueGroupNode Scene::CreateGroupNode()
{
    return ObjectAccess::MakeUnique<GroupNode>(nullptr, GetUniqueID());
}

UniqueTranslateNode Scene::CreateTranslateNode(Float3 distance)
{
    return ObjectAccess::MakeUnique<TranslateNode>(nullptr, GetUniqueID(), distance);
}

UniqueRotateNode Scene::CreateRotateNode(Float3 axis, Radians angle)
{
    return ObjectAccess::MakeUnique<RotateNode>(nullptr, GetUniqueID(), axis, angle);
}

UniqueScaleNode Scene::CreateScaleNode(float factor)
{
    return ObjectAccess::MakeUnique<ScaleNode>(nullptr, GetUniqueID(), factor);
}

UniqueInstanceNode Scene::CreateInstanceNode(MeshPtr mesh, MaterialPtr material)
{
    return ObjectAccess::MakeUnique<InstanceNode>(nullptr, GetUniqueID(), mesh, material);
}

ShaderPtr Scene::CreateShader()
{
    m_shaders.push_back(ObjectAccess::MakeUnique<Shader>(GetUniqueID()));
    return m_shaders.back().get();
}

MeshPtr Scene::CreateMesh(AABB aabb, MeshVertices vertices, MeshIndices indices)
{
    m_meshes.push_back(ObjectAccess::MakeUnique<Mesh>(GetUniqueID(), aabb, std::move(vertices), std::move(indices)));
    return m_meshes.back().get();
}

json Scene::ToJson() const
{
    json json;

    auto s_view  = std::views::transform(m_shaders, [](auto& shader) { return shader.get(); });
    auto shaders = nlohmann::json(Shaders(s_view.begin(), s_view.end()));

    auto m_view = std::views::transform(m_meshes, [](auto& mesh) { return mesh.get(); });
    auto meshes = nlohmann::json(Meshes(m_view.begin(), m_view.end()));

    json["scene"]   = m_root->ToJson();
    json["shaders"] = shaders;
    json["meshes"]  = meshes;

    return json;
}

Scene::~Scene()
{}

PropertyValue Shader::GetProperty(std::string_view /*name*/) const
{
    return {}; // TODO
}

PropertyValue Shader::GetProperty(std::string_view /*primary*/, std::string_view /*alternative*/) const
{
    return {}; // TODO
}

std::vector<Property> Shader::GetProperties() const
{
    return {}; // TODO
}

bool Shader::SetProperty(std::string_view /*name*/, const PropertyValue& /*value*/)
{
    return false; // TODO
}

bool Shader::RemoveProperty(std::string_view name)
{
    return ObjectAccess::RemoveProperty<kFieldNames.size()>(*this, name);
}

MaterialPtr Shader::CreateMaterial()
{
    m_materials.push_back(ObjectAccess::MakeUnique<Material>(GetUniqueID()));
    return static_cast<MaterialPtr>(m_materials.back().get());
}

Materials Shader::GetMaterials() const
{
    auto view = std::views::transform(m_materials, [](auto& material) { return material.get(); });
    return Materials(view.begin(), view.end());
}

json Shader::ToJson() const
{
    json json;

    ObjectAccess::ThisToJson(this, json);
    ObjectAccess::ChildrenToJson(m_materials, json);

    return json;
}

json Material::ToJson() const
{
    json json;

    ObjectAccess::ThisToJson(this, json);

    json["object.refs"] = m_instances;

    return json;
}

bool Material::RemoveInstance(InstanceNodePtr node)
{
    if (auto it = std::ranges::find(m_instances, node); it != m_instances.end()) {
        m_instances.erase(it);
        return true;
    }
    return false;
}

void Material::AddInstanceNodePtr(InstanceNodePtr instance_node)
{
    m_instances.push_back(instance_node);
}
