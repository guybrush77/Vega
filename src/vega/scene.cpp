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

namespace std {

static void to_json(json& json, const Dictionary& dictionary)
{
    for (const auto& [key, value] : dictionary) {
        visit(ValueToJson{ json, key }, value);
    }
}

} // namespace std

namespace glm {

static void to_json(json& json, const glm::vec3& vec)
{
    json["x"] = vec.x;
    json["y"] = vec.y;
    json["z"] = vec.z;
}

} // namespace glm

static void to_json(json& json, const UniqueNode& node)
{
    json = node->ToJson();
}

static void to_json(json& json, InstanceGeoNodePtr mesh)
{
    json = mesh->ID();
}

static void to_json(json& json, Radians radians)
{
    json["radians"] = radians.value;
}

static NodeID GetID() noexcept
{
    static std::atomic_int id = 0;
    id++;
    return id;
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

    static void UpdateInstances(GeoNodePtr node) { node->UpdateInstances(); }

    template <typename T>
    static void ThisToJson(const T* node, json& json)
    {
        json["node.class"] = typeid(*node).name();
        json["node.id"]    = node->m_id;

        if (node->HasProperties()) {
            json["node.properties"] = node->Node::ToJson();
        }
    }

    static void ChildrenToJson(const std::vector<UniqueNode>& nodes, json& json) { json["subset"] = nodes; }
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

InstanceGeoNodePtr InternalGeoNode::AddInstanceNode(InstanceMaterialNodePtr material_instance)
{
    assert(material_instance);
    m_nodes.push_back(NodeAccess::MakeUnique<InstanceGeoNode>(GetID(), material_instance));
    return static_cast<InstanceGeoNodePtr>(m_nodes.back().get());
}

NodeArray InternalGeoNode::GetChildren() const
{
    return NodeAccess::GetChildren(this);
}

Scene::Scene()
{
    m_materials = NodeAccess::MakeUnique<RootMaterialNode>(GetID());
    m_geometry  = NodeAccess::MakeUnique<RootGeoNode>(GetID());
}

void Scene::UpdateInstances() noexcept
{
    NodeAccess::UpdateInstances(static_cast<GeoNodePtr>(m_geometry.get()));
}

json RootGeoNode::ToJson() const
{
    json json;

    NodeAccess::ThisToJson(this, json);
    NodeAccess::ChildrenToJson(m_nodes, json);

    return json;
}

void RootGeoNode::UpdateInstances() noexcept
{
    for (auto& node : m_nodes) {
        NodeAccess::UpdateInstances(static_cast<GeoNodePtr>(node.get()));
    }
}

json InstanceGeoNode::ToJson() const
{
    json json;

    NodeAccess::ThisToJson(this, json);

    json["node.material.instance"] = m_material_instance->ID();

    return json;
}

void InstanceGeoNode::UpdateInstances() noexcept
{}

InstanceGeoNode::InstanceGeoNode(NodeID id, InstanceMaterialNodePtr material_instance) noexcept
    : GeoNode(id), m_material_instance(material_instance)
{
    NodeAccess::AddMeshInstancePtr(this, material_instance);
}

json TranslateGeoNode::ToJson() const
{
    json json;

    NodeAccess::ThisToJson(this, json);
    NodeAccess::ChildrenToJson(m_nodes, json);

    json["node.translate"] = m_distance;

    return json;
}

void TranslateGeoNode::UpdateInstances() noexcept
{
    for (auto& node : m_nodes) {
        NodeAccess::UpdateInstances(static_cast<GeoNodePtr>(node.get()));
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

void RotateGeoNode::UpdateInstances() noexcept
{
    for (auto& node : m_nodes) {
        NodeAccess::UpdateInstances(static_cast<GeoNodePtr>(node.get()));
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

void ScaleGeoNode::UpdateInstances() noexcept
{
    for (auto& node : m_nodes) {
        NodeAccess::UpdateInstances(static_cast<GeoNodePtr>(node.get()));
    }
}

const Value& Node::GetProperty(const Key& key) const noexcept
{
    if (m_dictionary) {
        if (auto it = m_dictionary->find(key); it != m_dictionary->end()) {
            return it->second;
        }
    }
    return nullvalue;
}

bool Node::HasProperty(const Key& key) const noexcept
{
    return m_dictionary ? m_dictionary->count(key) : false;
}

bool Node::HasProperties() const noexcept
{
    return m_dictionary && !m_dictionary->empty();
}

void Node::SetProperty(Key key, int value)
{
    SetPropertyPrivate(std::move(key), value);
}

void Node::SetProperty(Key key, float value)
{
    SetPropertyPrivate(std::move(key), value);
}

void Node::SetProperty(Key key, std::string value)
{
    SetPropertyPrivate(std::move(key), std::move(value));
}

void Node::RemoveProperty(Key key)
{
    if (m_dictionary) {
        m_dictionary->erase(key);
    }
}

json Node::ToJson() const
{
    return (m_dictionary && !m_dictionary->empty()) ? json(*m_dictionary) : json{};
}

void Node::SetPropertyPrivate(Key key, Value value)
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

    return json;
}

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

    json["node.geo.instances"] = m_mesh_instances;

    return json;
}
