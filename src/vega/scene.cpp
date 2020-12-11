#include "scene.hpp"

namespace {

struct ValueToJson final {
    void operator()(int i) { json[key] = i; }
    void operator()(float f) { json[key] = f; }
    void operator()(const std::string& s) { json[key] = s; }

    json&              json;
    const std::string& key;
};

} // namespace

namespace glm {

static void to_json(json& json, const glm::vec3& vec)
{
    json["x"] = vec.x;
    json["y"] = vec.y;
    json["z"] = vec.z;
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

static void to_json(json& json, InstanceGeoNodePtr mesh)
{
    json = mesh->ID();
}

static void to_json(json& json, Radians radians)
{
    json["radians"] = radians.value;
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

struct MeshStructure final {
    const MeshVertices* vertices;
    const MeshIndices*  indices;
};

static void to_json(json& json, const MeshStructure& geometry)
{
    json["vertices"] = *geometry.vertices;
    json["indices"]  = *geometry.indices;
}

static ID GetID() noexcept
{
    static std::atomic_int id = 0;
    id++;
    return { id };
}

struct NodeAccess {
    template <typename T, typename... Args>
    static auto MakeUnique(Args&&... args)
    {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }

    template <typename T>
    static auto GetChildren(const T* parent)
    {
        NodeArray ret;
        ret.reserve(parent->m_nodes.size());
        for (const auto& node : parent->m_nodes) {
            ret.push_back(node.get());
        }
        return ret;
    }

    static void AddMeshInstancePtr(InstanceGeoNodePtr mesh_instance, InstanceMaterialNodePtr material_instance)
    {
        assert(mesh_instance && material_instance);
        material_instance->AddMeshInstancePtr(mesh_instance);
    }

    static void ApplyTransform(GeoNodePtr node, const glm::mat4& matrix) { node->ApplyTransform(matrix); }

    template <typename T>
    static void ThisToJson(const T* node, json& json)
    {
        json["node.class"] = typeid(*node).name();
        json["node.id"]    = node->m_id;

        if (node->HasProperties()) {
            json["node.properties"] = node->PropertyDictionary::ToJson();
        }
    }

    static void ChildrenToJson(const std::vector<UniqueNode>& nodes, json& json) { json["sublist"] = nodes; }
};

struct MeshManager {
    MeshPtr CreateMesh(AABB aabb, MeshVertices mesh_vertices, MeshIndices mesh_indices)
    {
        auto mesh         = UniqueMesh(new Mesh(GetID(), aabb, std::move(mesh_vertices), std::move(mesh_indices)));
        auto mesh_id      = mesh->ID();
        auto mesh_ptr     = mesh.get();
        m_meshes[mesh_id] = std::move(mesh);

        return mesh_ptr;
    }

    MeshList GetNewMeshesAndReset()
    {
        MeshList mesh_list;
        for (const auto& [id, mesh] : m_meshes) {
            if (mesh->GetIsNewThenReset()) {
                mesh_list.push_back(id);
            }
        }
        return mesh_list;
    }

    MeshPtr GetMesh(MeshID mesh_id)
    {
        if (auto iter = m_meshes.find(mesh_id); iter != m_meshes.end()) {
            return iter->second.get();
        }
        return nullptr;
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
    std::unordered_map<MeshID, UniqueMesh, MeshID::Hash> m_meshes;
};

TranslateGeoNodePtr InternalGeoNode::AddTranslateNode(glm::vec3 amount)
{
    m_nodes.push_back(NodeAccess::MakeUnique<TranslateGeoNode>(GetID(), amount));
    return static_cast<TranslateGeoNodePtr>(m_nodes.back().get());
}

RotateGeoNodePtr InternalGeoNode::AddRotateNode(glm::vec3 axis, Radians angle)
{
    m_nodes.push_back(NodeAccess::MakeUnique<RotateGeoNode>(GetID(), axis, angle));
    return static_cast<RotateGeoNodePtr>(m_nodes.back().get());
}

ScaleGeoNodePtr InternalGeoNode::AddScaleNode(float factor)
{
    m_nodes.push_back(NodeAccess::MakeUnique<ScaleGeoNode>(GetID(), factor));
    return static_cast<ScaleGeoNodePtr>(m_nodes.back().get());
}

InstanceGeoNodePtr InternalGeoNode::AddInstanceNode(MeshPtr mesh, InstanceMaterialNodePtr material)
{
    assert(mesh && material);
    m_nodes.push_back(NodeAccess::MakeUnique<InstanceGeoNode>(GetID(), mesh, material));
    return static_cast<InstanceGeoNodePtr>(m_nodes.back().get());
}

NodeArray InternalGeoNode::GetChildren() const
{
    return NodeAccess::GetChildren(this);
}

Scene::Scene()
{
    m_materials    = NodeAccess::MakeUnique<RootMaterialNode>(GetID());
    m_geometry     = NodeAccess::MakeUnique<RootGeoNode>(GetID());
    m_mesh_manager = std::make_unique<MeshManager>();
}

DrawData Scene::GetDrawData() noexcept
{
    auto geo_root = static_cast<GeoNodePtr>(m_geometry.get());

    NodeAccess::ApplyTransform(geo_root, glm::identity<glm::mat4>());

    auto draw_data = DrawData{ .new_meshes = m_mesh_manager->GetNewMeshesAndReset() };

    return draw_data;
}

json RootGeoNode::ToJson() const
{
    json json;

    NodeAccess::ThisToJson(this, json);
    NodeAccess::ChildrenToJson(m_nodes, json);

    return json;
}

void RootGeoNode::ApplyTransform(const glm::mat4& matrix) noexcept
{
    for (auto& node : m_nodes) {
        NodeAccess::ApplyTransform(static_cast<GeoNodePtr>(node.get()), matrix);
    }
}

json InstanceGeoNode::ToJson() const
{
    json json;

    NodeAccess::ThisToJson(this, json);

    json["node.ref.material"] = m_material_instance->ID();
    json["node.ref.mesh"]     = m_mesh_instance->ID();
    json["node.transform"]    = m_transform;

    return json;
}

void InstanceGeoNode::ApplyTransform(const glm::mat4& matrix) noexcept
{
    m_transform = matrix;
}

InstanceGeoNode::InstanceGeoNode(NodeID id, MeshPtr mesh, InstanceMaterialNodePtr material) noexcept
    : GeoNode(id), m_mesh_instance(mesh), m_material_instance(material)
{
    NodeAccess::AddMeshInstancePtr(this, material);
}

json TranslateGeoNode::ToJson() const
{
    json json;

    NodeAccess::ThisToJson(this, json);
    NodeAccess::ChildrenToJson(m_nodes, json);

    json["node.translate"] = m_distance;

    return json;
}

void TranslateGeoNode::ApplyTransform(const glm::mat4& matrix) noexcept
{
    for (auto& node : m_nodes) {
        auto geo_node = static_cast<GeoNodePtr>(node.get());
        NodeAccess::ApplyTransform(geo_node, matrix * glm::translate(m_distance));
    }
}

json RotateGeoNode::ToJson() const
{
    json json;

    NodeAccess::ThisToJson(this, json);
    NodeAccess::ChildrenToJson(m_nodes, json);

    json["node.rotate.axis"]  = m_axis;
    json["node.rotate.angle"] = m_angle;

    return json;
}

void RotateGeoNode::ApplyTransform(const glm::mat4& matrix) noexcept
{
    for (auto& node : m_nodes) {
        auto geo_node = static_cast<GeoNodePtr>(node.get());
        NodeAccess::ApplyTransform(geo_node, matrix * glm::rotate(m_angle.value, m_axis));
    }
}

json ScaleGeoNode::ToJson() const
{
    json json;

    NodeAccess::ThisToJson(this, json);
    NodeAccess::ChildrenToJson(m_nodes, json);

    json["node.scale"] = m_factor;

    return json;
}

void ScaleGeoNode::ApplyTransform(const glm::mat4& matrix) noexcept
{
    for (auto& node : m_nodes) {
        auto geo_node = static_cast<GeoNodePtr>(node.get());
        NodeAccess::ApplyTransform(geo_node, matrix * glm::scale(glm::vec3{ m_factor, m_factor, m_factor }));
    }
}

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

RootGeoNodePtr Scene::GetGeometryRoot() noexcept
{
    return static_cast<RootGeoNodePtr>(m_geometry.get());
}

json Scene::ToJson() const
{
    json json;

    json["materials"] = m_materials->ToJson();
    json["geometry"]  = m_geometry->ToJson();
    json["meshes"]    = m_mesh_manager->ToJson();

    return json;
}

MeshPtr Scene::CreateMesh(AABB aabb, MeshVertices mesh_vertices, MeshIndices mesh_indices)
{
    return m_mesh_manager->CreateMesh(aabb, std::move(mesh_vertices), std::move(mesh_indices));
}

Scene::~Scene()
{}

RootMaterialNodePtr Scene::GetMaterialRoot() noexcept
{
    return static_cast<RootMaterialNodePtr>(m_materials.get());
}

InstanceMaterialNodePtr ClassMaterialNode::AddMaterialInstanceNode()
{
    m_nodes.push_back(NodeAccess::MakeUnique<InstanceMaterialNode>(GetID()));
    return static_cast<InstanceMaterialNodePtr>(m_nodes.back().get());
}

NodeArray ClassMaterialNode::GetChildren() const
{
    return NodeAccess::GetChildren(this);
}

json ClassMaterialNode::ToJson() const
{
    json json;

    NodeAccess::ThisToJson(this, json);
    NodeAccess::ChildrenToJson(m_nodes, json);

    return json;
}

ClassMaterialNodePtr RootMaterialNode::AddMaterialClassNode()
{
    m_nodes.push_back(NodeAccess::MakeUnique<ClassMaterialNode>(GetID()));
    return static_cast<ClassMaterialNodePtr>(m_nodes.back().get());
}

NodeArray RootMaterialNode::GetChildren() const
{
    return NodeAccess::GetChildren(this);
}

json RootMaterialNode::ToJson() const
{
    json json;

    NodeAccess::ThisToJson(this, json);
    NodeAccess::ChildrenToJson(m_nodes, json);

    return json;
}

json InstanceMaterialNode::ToJson() const
{
    json json;

    NodeAccess::ThisToJson(this, json);

    json["node.refs.geo"] = m_mesh_instances;

    return json;
}

MeshPtr Scene::GetMesh(MeshID mesh_id)
{
    return m_mesh_manager->GetMesh(mesh_id);
}

bool Mesh::GetIsNewThenReset() noexcept
{
    return std::exchange(m_is_new, false);
}

json Mesh::ToJson() const
{
    json json;

    json["mesh.id"]        = m_id;
    json["mesh.span"]      = m_aabb;
    json["mesh.structure"] = MeshStructure{ &m_vertices, &m_indices };

    if (HasProperties()) {
        json["mesh.properties"] = PropertyDictionary::ToJson();
    }

    return json;
}
