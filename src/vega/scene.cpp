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

namespace glm {

static void to_json(json& json, const glm::vec3& vec)
{
    json = { vec.x, vec.y, vec.z };
}

static void to_json(json& json, const glm::mat4& m)
{
    float a[16];

    memcpy(a, &m[0][0], 16 * sizeof(float));

    json = nlohmann::json(a);
}

} // namespace glm

static void to_json(json& json, const UniqueNode& node)
{
    json = node->ToJson();
}

static void to_json(json& json, const AABB& aabb)
{
    json["min"] = aabb.min;
    json["max"] = aabb.max;
}

static void to_json(json& json, ID id)
{
    json = id.value;
}

static void to_json(json& json, MeshNodePtr mesh_node)
{
    json = mesh_node->GetID();
}

static void to_json(json& json, const Mesh& mesh)
{
    json = mesh.ToJson();
}

static void to_json(json& json, const MeshVertices& mesh_vertices)
{
    json["vertex.type"]   = mesh_vertices.Type();
    json["vertex.size"]   = mesh_vertices.VertexSize();
    json["vertex.count"]  = mesh_vertices.Count();
    json["vertices.size"] = mesh_vertices.Size();
}

static void to_json(json& json, const MeshIndices& mesh_indices)
{
    json["index.type"]   = mesh_indices.Type();
    json["index.size"]   = mesh_indices.IndexSize();
    json["index.count"]  = mesh_indices.Count();
    json["indices.size"] = mesh_indices.Size();
}

struct RotateValues final {
    const glm::vec3 axis;
    const float     angle;
};

static void to_json(json& json, const RotateValues& values)
{
    json["rotate.axis"]  = values.axis;
    json["rotate.angle"] = values.angle;
}

struct TranslateValues final {
    const glm::vec3 distance;
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

struct InstantiateValues final {
    const glm::mat4* transform;
};

static void to_json(json& json, const InstantiateValues& values)
{
    json["transform"] = *values.transform;
}

struct InstantiateRefs final {
    const ID material_instance;
    const ID mesh;
};

static void to_json(json& json, const InstantiateRefs& values)
{
    json["material.instance"] = values.material_instance;
    json["mesh"]              = values.mesh;
}

struct MeshStructure final {
    const MeshVertices* vertices;
    const MeshIndices*  indices;
};

static void to_json(json& json, const MeshStructure& geometry)
{
    json["vertices"] = *geometry.vertices;
    json["indices"]  = *geometry.indices;
}

json Mesh::ToJson() const
{
    json json;

    json["mesh.id"]        = m_id;
    json["mesh.span"]      = m_aabb;
    json["mesh.structure"] = MeshStructure{ &m_vertices, &m_indices };

    // TODO
    // if (HasProperties()) {
    //    json["mesh.properties"] = PropertyDictionary::ToJson();
    //}

    return json;
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
    static auto GetChildren(const T* parent)
    {
        NodeArray ret;
        ret.reserve(parent->m_children.size());
        for (const auto& node : parent->m_children) {
            ret.push_back(node.get());
        }
        return ret;
    }

    static void AddMeshInstancePtr(MeshNodePtr object_instance, MaterialInstancePtr material_instance)
    {
        assert(object_instance && material_instance);
        // material_instance->AddInstantiateObjectNodePtr(object_instance); // TODO
    }

    static void ApplyTransform(NodePtr node, const glm::mat4& matrix) { node->ApplyTransform(matrix); }

    template <typename T>
    static void ThisToJson(const T* node, json& json)
    {
        json["object.class"] = node->metadata.object_class;
        json["object.id"]    = node->m_id;

        if (node->HasProperties()) {
            // json["object.properties"] = node->PropertyDictionary::ToJson(); // TODO
        }
    }

    static void ChildrenToJson(const std::vector<UniqueNode>& nodes, json& json) { json["pack"] = nodes; }
};

class MaterialManager {
  public:
    MaterialPtr CreateMaterial()
    {
        auto material_id         = GetUniqueID();
        auto material            = UniqueMaterial(new Material(material_id));
        auto material_ptr        = material.get();
        m_materials[material_id] = std::move(material);

        return material_ptr;
    }

    json ToJson() const
    {
        /*
        auto mesh_refs = std::vector<std::reference_wrapper<Mesh>>{};
        mesh_refs.reserve(m_meshes.size());
        for (auto& pair : m_meshes) {
            mesh_refs.push_back(*pair.second);
        }
        return nlohmann::json(mesh_refs);
        */
        // TODO
        return {};
    }

  private:
    std::unordered_map<ID, UniqueMaterial, ID::Hash> m_materials;
};

class MeshManager {
  public:
    MeshPtr CreateMesh(AABB aabb, MeshVertices mesh_vertices, MeshIndices mesh_indices)
    {
        auto mesh     = UniqueMesh(new Mesh(GetUniqueID(), aabb, std::move(mesh_vertices), std::move(mesh_indices)));
        auto mesh_id  = mesh->GetID();
        auto mesh_ptr = mesh.get();
        m_meshes[mesh_id] = std::move(mesh);

        return mesh_ptr;
    }

    json ToJson() const
    {
        auto mesh_refs = std::vector<std::reference_wrapper<Mesh>>{};
        mesh_refs.reserve(m_meshes.size());
        for (auto& pair : m_meshes) {
            mesh_refs.push_back(*pair.second);
        }
        return nlohmann::json(mesh_refs);
    }

  private:
    std::unordered_map<ID, UniqueMesh, ID::Hash> m_meshes;
};

TranslateNodePtr InternalNode::AddTranslateNode(glm::vec3 amount)
{
    m_children.push_back(ObjectAccess::MakeUnique<TranslateNode>(GetUniqueID(), amount));
    return static_cast<TranslateNodePtr>(m_children.back().get());
}

RotateNodePtr InternalNode::AddRotateNode(glm::vec3 axis, Radians angle)
{
    m_children.push_back(ObjectAccess::MakeUnique<RotateNode>(GetUniqueID(), axis, angle));
    return static_cast<RotateNodePtr>(m_children.back().get());
}

ScaleNodePtr InternalNode::AddScaleNode(float factor)
{
    m_children.push_back(ObjectAccess::MakeUnique<ScaleNode>(GetUniqueID(), factor));
    return static_cast<ScaleNodePtr>(m_children.back().get());
}

MeshNodePtr InternalNode::AddMeshNode(MeshPtr mesh, MaterialInstancePtr material)
{
    assert(mesh && material);
    m_children.push_back(ObjectAccess::MakeUnique<MeshNode>(GetUniqueID(), mesh, material));
    return static_cast<MeshNodePtr>(m_children.back().get());
}

NodeArray InternalNode::GetChildren() const
{
    return ObjectAccess::GetChildren(this);
}

Scene::Scene()
{
    // m_materials    = ObjectAccess::MakeUnique<RootMaterialNode>(GetUniqueID()); // TODO
    m_root_node    = ObjectAccess::MakeUnique<RootNode>(GetUniqueID());
    m_mesh_manager = std::make_unique<MeshManager>();
}

DrawList Scene::ComputeDrawList() const
{
    using namespace std::ranges;
    /*
    auto root = static_cast<NodePtr>(m_root_node.get());

    ObjectAccess::ApplyTransform(root, glm::identity<glm::mat4>());

    auto draw_list        = DrawList{};
    auto material_classes = m_materials->GetChildren();

    for (auto material_class : material_classes | views::transform(pointer_cast<MaterialClassPtr>)) {
        auto material_instances = material_class->GetChildren();

        for (auto material_instance : material_instances | views::transform(pointer_cast<MaterialInstancePtr>)) {
            auto mesh_instances = material_instance->GetChildren();

            for (auto mesh_instance : mesh_instances | views::transform(pointer_cast<MeshNodePtr>)) {
                draw_list.push_back({ mesh_instance->GetMeshPtr(), mesh_instance->GetTransformPtr() });
            }
        }
    }
    */
    // TODO
    // return draw_list;
    return {};
}

AABB Scene::ComputeAxisAlignedBoundingBox() const
{
    using namespace std::ranges;
    /*
    auto out = AABB{ { FLT_MAX, FLT_MAX, FLT_MAX }, { FLT_MIN, FLT_MIN, FLT_MIN } };

    auto root = static_cast<ObjectNodePtr>(m_root_node.get());

    ObjectAccess::ApplyTransform(root, glm::identity<glm::mat4>());

    auto draw_list        = DrawList{};
    auto material_classes = m_materials->GetChildren();

    for (auto material_class : material_classes | views::transform(pointer_cast<MaterialClassPtr>)) {
        auto material_instances = material_class->GetChildren();

        for (auto material_instance : material_instances | views::transform(pointer_cast<MaterialInstancePtr>)) {
            auto mesh_instances = material_instance->GetChildren();

            for (auto mesh_instance : mesh_instances | views::transform(pointer_cast<InstantiateObjectNodePtr>)) {
                if (auto* mesh = mesh_instance->GetMeshPtr()) {
                    auto& model = *mesh_instance->GetTransformPtr();
                    auto& aabb  = mesh->GetAxisAlignedBoundingBox();
                    auto  vec_a = model * glm::vec4(aabb.min, 1);
                    auto  vec_b = model * glm::vec4(aabb.max, 1);
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
    */

    // TODO
    return {};
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

json MeshNode::ToJson() const
{
    json json;

    ObjectAccess::ThisToJson(this, json);

    json["node.refs"]   = InstantiateRefs{ {} /*m_material_instance->GetID()*/, m_mesh->GetID() }; // TODO
    json["node.values"] = InstantiateValues{ &m_transform };

    return json;
}

void MeshNode::ApplyTransform(const glm::mat4& matrix) noexcept
{
    m_transform = matrix;
}

MeshNode::MeshNode(ID id, MeshPtr mesh, MaterialInstancePtr material) noexcept
    : Node(id), m_mesh(mesh), m_material_instance(material)
{
    ObjectAccess::AddMeshInstancePtr(this, material);
}

json TranslateNode::ToJson() const
{
    json json;

    ObjectAccess::ThisToJson(this, json);
    ObjectAccess::ChildrenToJson(m_children, json);

    json["node.values"] = TranslateValues{ m_amount };

    return json;
}

void TranslateNode::ApplyTransform(const glm::mat4& matrix) noexcept
{
    for (auto& node : m_children) {
        auto node_ptr = static_cast<NodePtr>(node.get());
        ObjectAccess::ApplyTransform(node_ptr, matrix * glm::translate(m_amount));
    }
}

json RotateNode::ToJson() const
{
    json json;

    ObjectAccess::ThisToJson(this, json);
    ObjectAccess::ChildrenToJson(m_children, json);

    json["node.values"] = RotateValues{ m_axis, m_angle.value };

    return json;
}

void RotateNode::ApplyTransform(const glm::mat4& matrix) noexcept
{
    for (auto& node : m_children) {
        auto node_ptr = static_cast<NodePtr>(node.get());
        ObjectAccess::ApplyTransform(node_ptr, matrix * glm::rotate(m_angle.value, m_axis));
    }
}

json ScaleNode::ToJson() const
{
    json json;

    ObjectAccess::ThisToJson(this, json);
    ObjectAccess::ChildrenToJson(m_children, json);

    json["node.values"] = ScaleValues{ m_factor };

    return json;
}

void ScaleNode::ApplyTransform(const glm::mat4& matrix) noexcept
{
    for (auto& node : m_children) {
        auto node_ptr = static_cast<NodePtr>(node.get());
        ObjectAccess::ApplyTransform(node_ptr, matrix * glm::scale(glm::vec3{ m_factor, m_factor, m_factor }));
    }
}

/*
const Value& PropertyDictionary::GetProperty(const Key& key) const noexcept
{
    if (m_dictionary) {
        if (auto it = m_dictionary->find(key); it != m_dictionary->end()) {
            return it->second;
        }
    }
    return nullvalue;
}

bool PropertyDictionary::HasProperty(const Key& key) const noexcept
{
    return m_dictionary ? m_dictionary->count(key) : false;
}

bool PropertyDictionary::HasProperties() const noexcept
{
    return m_dictionary && !m_dictionary->empty();
}

void PropertyDictionary::SetProperty(Key key, int value)
{
    SetPropertyPrivate(std::move(key), value);
}

void PropertyDictionary::SetProperty(Key key, float value)
{
    SetPropertyPrivate(std::move(key), value);
}

void PropertyDictionary::SetProperty(Key key, std::string value)
{
    SetPropertyPrivate(std::move(key), std::move(value));
}

void PropertyDictionary::RemoveProperty(Key key)
{
    if (m_dictionary) {
        m_dictionary->erase(key);
    }
}

json PropertyDictionary::ToJson() const
{
    json json;

    for (const auto& [key, value] : *m_dictionary) {
        std::visit(ValueToJson{ json, key }, value);
    }

    return json;
}

void PropertyDictionary::SetPropertyPrivate(Key key, Value value)
{
    if (m_dictionary == nullptr) {
        m_dictionary.reset(new Dictionary);
    }
    m_dictionary->insert_or_assign(std::move(key), std::move(value));
}
*/

RootNodePtr Scene::GetRootNodePtr() noexcept
{
    return static_cast<RootNodePtr>(m_root_node.get());
}

MaterialPtr Scene::CreateMaterial()
{
    return m_material_manager->CreateMaterial();
}

json Scene::ToJson() const
{
    json json;

    // json["materials"] = m_materials->ToJson(); // TODO
    json["scene"]  = m_root_node->ToJson();
    json["meshes"] = m_mesh_manager->ToJson();

    return json;
}

MeshPtr Scene::CreateMesh(AABB aabb, MeshVertices mesh_vertices, MeshIndices mesh_indices)
{
    return m_mesh_manager->CreateMesh(aabb, std::move(mesh_vertices), std::move(mesh_indices));
}

Scene::~Scene()
{}

/*
MaterialInstancePtr ClassMaterialNode::AddMaterialInstanceNode()
{
    m_children.push_back(ObjectAccess::MakeUnique<InstanceMaterialNode>(GetUniqueID()));
    return static_cast<MaterialInstancePtr>(m_children.back().get());
}

NodeArray ClassMaterialNode::GetChildren() const
{
    return ObjectAccess::GetChildren(this);
}
*/

json Material::ToJson() const
{
    json json;

    ObjectAccess::ThisToJson(this, json);
    ObjectAccess::ChildrenToJson(m_children, json);

    return json;
}
/*
* 
MaterialClassPtr RootMaterialNode::AddMaterialClassNode()
{
    m_children.push_back(ObjectAccess::MakeUnique<ClassMaterialNode>(GetUniqueID()));
    return static_cast<MaterialClassPtr>(m_children.back().get());
}

NodeArray RootMaterialNode::GetChildren() const
{
    return ObjectAccess::GetChildren(this);
}

json RootMaterialNode::ToJson() const
{
    json json;

    ObjectAccess::ThisToJson(this, json);
    ObjectAccess::ChildrenToJson(m_children, json);

    return json;
}

json InstanceMaterialNode::ToJson() const
{
    json json;

    ObjectAccess::ThisToJson(this, json);

    auto refs = std::vector<InstantiateObjectNodePtr>(m_children.size());

    std::ranges::transform(m_children, refs.begin(), pointer_cast<InstantiateObjectNodePtr>);

    json["node.refs.geo"] = refs;

    return json;
}

void InstanceMaterialNode::AddInstantiateObjectNodePtr(InstantiateObjectNodePtr object_instance)
{
    m_children.push_back(static_cast<NodePtr>(object_instance));
}
*/
