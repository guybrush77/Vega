#include "scene.hpp"

#include "utils/cast.hpp"

#include <atomic>
#include <ranges>

namespace {

struct ValueToJson final {
    void operator()(int i) { j[key] = i; }
    void operator()(float f) { j[key] = f; }
    void operator()(const std::string& s) { j[key] = s; }

    json&              j;
    const std::string& key;
};

} // namespace

static void to_json(json& json, const Float3& vec)
{
    json = { vec.x, vec.y, vec.z };
}

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

static void to_json(json& json, ID id)
{
    json = id.value;
}

static void to_json(json& json, const Shader& material)
{
    json = material.ToJson();
}

static void to_json(json& json, InstanceNodePtr instance_node)
{
    json = instance_node->GetID();
}

static void to_json(json& json, const Mesh& mesh)
{
    json = mesh.ToJson();
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

struct DictionaryValues final {
    const Dictionary* dictionary = nullptr;
};

static void to_json(json& json, const DictionaryValues& values)
{
    for (const auto& [key, value] : *values.dictionary) {
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
    const Float3 amount;
};

static void to_json(json& json, const TranslateValues& values)
{
    json["translate"] = values.amount;
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

struct ObjectAccess {
    template <typename T, typename... Args>
    static auto MakeUnique(Args&&... args)
    {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }

    template <typename T>
    static auto HasChildren(const T* parent)
    {
        return !parent->m_children.empty();
    }

    template <typename T>
    static auto GetChildren(const T* parent)
    {
        NodePtrArray ret;
        ret.reserve(parent->m_children.size());
        for (const auto& node : parent->m_children) {
            ret.push_back(node.get());
        }
        return ret;
    }

    static void AddInstancePtr(InstanceNodePtr instance_node, MaterialPtr material)
    {
        assert(instance_node && material);
        material->AddInstanceNodePtr(instance_node);
    }

    static void AttachNode(InternalNode* parent, UniqueNode child)
    {
        assert(parent && child);
        child->m_parent = parent;
        parent->m_children.push_back(std::move(child));
    }

    static UniqueNode DetachNode(NodePtr node)
    {
        assert(node);

        auto parent = static_cast<InternalNode*>(node->m_parent);

        if (parent == nullptr) {
            return nullptr;
        }

        auto it = std::ranges::find_if(parent->m_children, [node](auto& child) { return child.get() == node; });

        if (it == parent->m_children.end()) {
            return nullptr;
        }

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
        json["object.class"] = object->metadata.object_class;
        json["object.id"]    = object->m_id;

        if (object->HasProperties()) {
            json["object.properties"] = DictionaryValues{ object->m_dictionary.get() };
        }
    }

    template <typename T>
    static void ChildrenToJson(const std::vector<T>& children, json& json)
    {
        json["owns"] = children;
    }
};

class ShaderManager {
  public:
    ShaderPtr CreateShader()
    {
        auto  id       = GetUniqueID();
        auto& material = m_shaders[id] = ObjectAccess::MakeUnique<Shader>(id);
        return material.get();
    }

    ShaderPtrArray GetShaders() const
    {
        auto view = std::views::transform(m_shaders, [](auto& shader) { return shader.second.get(); });
        return ShaderPtrArray(view.begin(), view.end());
    }

    json ToJson() const
    {
        auto view = std::views::transform(m_shaders, [](auto& shader) { return std::ref(*shader.second); });
        return nlohmann::json(ShaderRefArray(view.begin(), view.end()));
    }

  private:
    std::unordered_map<ID, UniqueShader, ID::Hash> m_shaders;
};

class MeshManager {
  public:
    MeshPtr CreateMesh(AABB aabb, MeshVertices vertices, MeshIndices indices)
    {
        auto  id   = GetUniqueID();
        auto& mesh = m_meshes[id] = ObjectAccess::MakeUnique<Mesh>(id, aabb, std::move(vertices), std::move(indices));
        return mesh.get();
    }

    json ToJson() const
    {
        auto view = std::views::transform(m_meshes, [](auto& mesh) { return std::ref(*mesh.second); });
        return nlohmann::json(MeshRefArray(view.begin(), view.end()));
    }

  private:
    std::unordered_map<ID, UniqueMesh, ID::Hash> m_meshes;
};

json Mesh::ToJson() const
{
    json json;

    ObjectAccess::ThisToJson(this, json);
    json["object.values"] = MeshValues{ &m_aabb, &m_vertices, &m_indices };

    return json;
}

ValueRef Mesh::GetField(std::string_view field_name)
{
    if (field_name == "aabb.min") {
        return ValueRef(&m_aabb.min);
    } else if (field_name == "aabb.max") {
        return ValueRef(&m_aabb.max);
    } else if (field_name == "vertex.attributes") {
        return ValueRef(&m_vertices.VertexAttributesRef());
    } else if (field_name == "vertex.size") {
        return ValueRef(&m_vertices.VertexSizeRef());
    } else if (field_name == "vertex.count") {
        return ValueRef(&m_vertices.CountRef());
    } else if (field_name == "index.type") {
        return ValueRef(&m_indices.IndexTypeRef());
    } else if (field_name == "index.size") {
        return ValueRef(&m_indices.IndexSizeRef());
    } else if (field_name == "index.count") {
        return ValueRef(&m_indices.CountRef());
    }
    return {};
}

Metadata Mesh::metadata = {
    "mesh",
    "Mesh",
    "Mesh",
    { Field{ "aabb.min", "Min", nullptr, ValueType::Float3, Field::IsEditable{ false } },
      Field{ "aabb.max", "Max", nullptr, ValueType::Float3, Field::IsEditable{ false } },
      Field{ "vertex.attributes", "Vertex Attributes", nullptr, ValueType::String, Field::IsEditable{ false } },
      Field{ "vertex.size", "Vertex Size", nullptr, ValueType::Int, Field::IsEditable{ false } },
      Field{ "vertex.count", "Vertex Count", nullptr, ValueType::Int, Field::IsEditable{ false } },
      Field{ "index.type", "Index Type", nullptr, ValueType::String, Field::IsEditable{ false } },
      Field{ "index.size", "Index Size", nullptr, ValueType::Int, Field::IsEditable{ false } },
      Field{ "index.count", "Index Count", nullptr, ValueType::Int, Field::IsEditable{ false } } }
};

GroupNodePtr InternalNode::AddGroupNode()
{
    m_children.push_back(ObjectAccess::MakeUnique<GroupNode>(this, GetUniqueID()));
    return static_cast<GroupNodePtr>(m_children.back().get());
}

TranslateNodePtr InternalNode::AddTranslateNode(float x, float y, float z)
{
    m_children.push_back(ObjectAccess::MakeUnique<TranslateNode>(this, GetUniqueID(), x, y, z));
    return static_cast<TranslateNodePtr>(m_children.back().get());
}

RotateNodePtr InternalNode::AddRotateNode(float x, float y, float z, Radians angle)
{
    m_children.push_back(ObjectAccess::MakeUnique<RotateNode>(this, GetUniqueID(), x, y, z, angle));
    return static_cast<RotateNodePtr>(m_children.back().get());
}

ScaleNodePtr InternalNode::AddScaleNode(float factor)
{
    m_children.push_back(ObjectAccess::MakeUnique<ScaleNode>(this, GetUniqueID(), factor));
    return static_cast<ScaleNodePtr>(m_children.back().get());
}

InstanceNodePtr InternalNode::AddInstanceNode(MeshPtr mesh, MaterialPtr material)
{
    assert(mesh && material);
    m_children.push_back(ObjectAccess::MakeUnique<InstanceNode>(this, GetUniqueID(), mesh, material));
    return static_cast<InstanceNodePtr>(m_children.back().get());
}

void InternalNode::AttachNode(UniqueNode node)
{
    ObjectAccess::AttachNode(this, std::move(node));
}

UniqueNode InternalNode::DetachNode()
{
    return ObjectAccess::DetachNode(this);
}

bool InternalNode::IsAncestor(NodePtr node) const
{
    return ObjectAccess::IsAncestor(this, node);
}

bool InternalNode::HasChildren() const
{
    return ObjectAccess::HasChildren(this);
}

NodePtrArray InternalNode::GetChildren() const
{
    return ObjectAccess::GetChildren(this);
}

Scene::Scene()
{
    m_shader_manager = std::make_unique<ShaderManager>();
    m_mesh_manager   = std::make_unique<MeshManager>();
    m_root_node      = ObjectAccess::MakeUnique<RootNode>(nullptr, GetUniqueID());
}

DrawList Scene::ComputeDrawList() const
{
    using namespace std::ranges;

    ObjectAccess::ApplyTransform(m_root_node.get(), glm::identity<glm::mat4>());

    auto draw_list = DrawList{};

    for (auto shader : m_shader_manager->GetShaders()) {
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

    if (m_root_node->GetChildren().empty()) {
        return AABB{ { -1, -1, -1 }, { 1, 1, 1 } };
    }

    ObjectAccess::ApplyTransform(m_root_node.get(), glm::identity<glm::mat4>());

    auto out = AABB{ { FLT_MAX, FLT_MAX, FLT_MAX }, { FLT_MIN, FLT_MIN, FLT_MIN } };

    for (auto shader : m_shader_manager->GetShaders()) {
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

Metadata RootNode::metadata = { "root.node", "Root", nullptr, {} };

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

Metadata GroupNode::metadata = { "group.node", "Group", nullptr, {} };

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

UniqueNode InstanceNode::DetachNode()
{
    return ObjectAccess::DetachNode(this);
}

InstanceNode::~InstanceNode() noexcept
{
    if (m_material) {
        m_material->RemoveInstance(this);
    }
}

ValueRef InstanceNode::GetField(std::string_view field_name)
{
    if (field_name == "mesh") {
        return ValueRef(m_mesh);
    } else if (field_name == "material") {
        return ValueRef(m_material);
    }
    return {};
}

Metadata InstanceNode::metadata = {
    "instance.node",
    "Mesh Instance",
    nullptr,
    { Field{ "mesh", "Mesh", nullptr, ValueType::Reference, Field::IsEditable{ false } },
      Field{ "material", "Material", nullptr, ValueType::Reference, Field::IsEditable{ false } } }
};

void InstanceNode::ApplyTransform(const glm::mat4& matrix) noexcept
{
    m_transform = matrix;
}

InstanceNode::InstanceNode(NodePtr parent, ID id, MeshPtr mesh, MaterialPtr material) noexcept
    : Node(parent, id), m_mesh(mesh), m_material(material)
{
    ObjectAccess::AddInstancePtr(this, material);
}

ValueRef TranslateNode::GetField(std::string_view field_name)
{
    return field_name == "translate.amount" ? ValueRef(&m_amount) : ValueRef();
}

Metadata TranslateNode::metadata = {
    "translate.node",
    "Translate",
    "Translate Node",
    { Field{ "translate.amount", "Amount", nullptr, ValueType::Float3, Field::IsEditable{ true } } }
};

json TranslateNode::ToJson() const
{
    json json;

    ObjectAccess::ThisToJson(this, json);
    ObjectAccess::ChildrenToJson(m_children, json);

    json["object.values"] = TranslateValues{ m_amount };

    return json;
}

void TranslateNode::ApplyTransform(const glm::mat4& matrix) noexcept
{
    for (auto& node : m_children) {
        auto node_ptr = static_cast<NodePtr>(node.get());
        auto amount   = glm::vec3(m_amount.x, m_amount.y, m_amount.z);
        ObjectAccess::ApplyTransform(node_ptr, matrix * glm::translate(amount));
    }
}

json RotateNode::ToJson() const
{
    json json;

    ObjectAccess::ThisToJson(this, json);
    ObjectAccess::ChildrenToJson(m_children, json);

    json["object.values"] = RotateValues{ m_axis, m_angle.value };

    return json;
}

ValueRef RotateNode::GetField(std::string_view field_name)
{
    if (field_name == "rotate.axis") {
        return ValueRef(&m_axis);
    } else if (field_name == "rotate.angle") {
        return ValueRef(&m_angle.value);
    }
    return {};
}

Metadata RotateNode::metadata = {
    "rotate.node",
    "Rotate",
    "Rotate Node",
    { Field{ "rotate.axis", "Axis", nullptr, ValueType::Float3, Field::IsEditable{ true } },
      Field{ "rotate.angle", "Angle", nullptr, ValueType::Float, Field::IsEditable{ true } } }
};

void RotateNode::ApplyTransform(const glm::mat4& matrix) noexcept
{
    for (auto& node : m_children) {
        auto node_ptr = static_cast<NodePtr>(node.get());
        auto axis     = glm::vec3(m_axis.x, m_axis.y, m_axis.z);
        ObjectAccess::ApplyTransform(node_ptr, matrix * glm::rotate(m_angle.value, axis));
    }
}

json ScaleNode::ToJson() const
{
    json json;

    ObjectAccess::ThisToJson(this, json);
    ObjectAccess::ChildrenToJson(m_children, json);

    json["object.values"] = ScaleValues{ m_factor };

    return json;
}

ValueRef ScaleNode::GetField(std::string_view field_name)
{
    return field_name == "scale.factor" ? ValueRef(&m_factor) : ValueRef();
}

void ScaleNode::ApplyTransform(const glm::mat4& matrix) noexcept
{
    for (auto& node : m_children) {
        auto node_ptr = static_cast<NodePtr>(node.get());
        ObjectAccess::ApplyTransform(node_ptr, matrix * glm::scale(glm::vec3{ m_factor, m_factor, m_factor }));
    }
}

Metadata ScaleNode::metadata = {
    "scale.node",
    "Scale",
    "Scale Node",
    { Field{ "scale.factor", "Factor", nullptr, ValueType::Float, Field::IsEditable{ true } } }
};

std::string Object::GetName() const noexcept
{
    if (m_dictionary) {
        if (auto it = m_dictionary->find("object.name"); it != m_dictionary->end()) {
            return std::get<std::string>(it->second).c_str();
        }
    }
    return GetMetadata().object_default_name;
}

bool Object::HasProperties() const noexcept
{
    return m_dictionary && !m_dictionary->empty();
}

void Object::SetName(std::string name)
{
    SetProperty("object.name", std::move(name));
}

void Object::SetProperty(Key key, Value value)
{
    if (m_dictionary == nullptr) {
        m_dictionary.reset(new Dictionary);
    }
    m_dictionary->insert_or_assign(std::move(key), std::move(value));
}

void Object::RemoveProperty(Key key)
{
    if (m_dictionary) {
        m_dictionary->erase(key);
    }
}

RootNodePtr Scene::GetRootNodePtr() noexcept
{
    return static_cast<RootNodePtr>(m_root_node.get());
}

ShaderPtr Scene::CreateShader()
{
    return m_shader_manager->CreateShader();
}

json Scene::ToJson() const
{
    json json;

    json["scene"]   = m_root_node->ToJson();
    json["shaders"] = m_shader_manager->ToJson();
    json["meshes"]  = m_mesh_manager->ToJson();

    return json;
}

MeshPtr Scene::CreateMesh(AABB aabb, MeshVertices mesh_vertices, MeshIndices mesh_indices)
{
    return m_mesh_manager->CreateMesh(aabb, std::move(mesh_vertices), std::move(mesh_indices));
}

Scene::~Scene()
{}

MaterialPtr Shader::CreateMaterial()
{
    m_materials.push_back(ObjectAccess::MakeUnique<Material>(GetUniqueID()));
    return static_cast<MaterialPtr>(m_materials.back().get());
}

MaterialPtrArray Shader::GetMaterials() const
{
    auto view = std::views::transform(m_materials, [](auto& material) { return material.get(); });
    return MaterialPtrArray(view.begin(), view.end());
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

    json["object.refs"] = m_instance_nodes;

    return json;
}

bool Material::RemoveInstance(InstanceNodePtr node)
{
    if (auto it = std::ranges::find(m_instance_nodes, node); it != m_instance_nodes.end()) {
        m_instance_nodes.erase(it);
        return true;
    }
    return false;
}

void Material::AddInstanceNodePtr(InstanceNodePtr instance_node)
{
    m_instance_nodes.push_back(instance_node);
}

std::string to_string(ValueType value_type)
{
    switch (value_type) {
    case ValueType::Null: return "Null";
    case ValueType::Float: return "Float";
    case ValueType::Int: return "Int";
    case ValueType::Reference: return "Reference";
    case ValueType::String: return "String";
    case ValueType::Float3: return "Float3";
    default: utils::throw_runtime_error("ValueType: Bad enum value");
    };
    return {};
}
