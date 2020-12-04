#pragma once

#include "utils/math.hpp"

#include <nlohmann/json.hpp>

#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

using json = nlohmann::json;

using Key        = std::string;
using Value      = std::variant<int, float, std::string>;
using KeyValue   = std::pair<const Key, Value>;
using Dictionary = std::unordered_map<Key, Value>;

struct Node;

using NodeID     = std::uint32_t;
using UniqueNode = std::unique_ptr<Node>;
using NodePtr    = Node*;
using NodeRef    = Node&;
using NodeArray  = std::vector<NodePtr>;

struct MaterialNode;
struct RootMaterialNode;
struct ClassMaterialNode;
struct InstanceMaterialNode;

using MaterialNodePtr         = MaterialNode*;
using RootMaterialNodePtr     = RootMaterialNode*;
using ClassMaterialNodePtr    = ClassMaterialNode*;
using InstanceMaterialNodePtr = InstanceMaterialNode*;

struct GeoNode;
struct RootGeoNode;
struct TranslateGeoNode;
struct RotateGeoNode;
struct ScaleGeoNode;
struct InstanceGeoNode;

using GeoNodePtr          = GeoNode*;
using RootGeoNodePtr      = RootGeoNode*;
using TranslateGeoNodePtr = TranslateGeoNode*;
using RotateGeoNodePtr    = RotateGeoNode*;
using ScaleGeoNodePtr     = ScaleGeoNode*;
using InstanceGeoNodePtr  = InstanceGeoNode*;

struct Node {
    auto GetProperty(const Key& key) const noexcept -> const Value&;
    bool HasProperty(const Key& key) const noexcept;
    bool HasProperties() const noexcept;
    void SetProperty(Key key, int value);
    void SetProperty(Key key, float value);
    void SetProperty(Key key, std::string value);
    void RemoveProperty(Key key);

    auto ID() const noexcept { return m_id; }

    virtual auto GetChildren() const -> NodeArray = 0;

    virtual json ToJson() const = 0;

    virtual ~Node() noexcept = default;

  protected:
    friend struct NodeAccess;

    Node(NodeID id) noexcept : m_id(id) {}

    void SetPropertyPrivate(Key key, Value value);

    inline static const Value nullvalue;

    NodeID                      m_id = 0;
    std::unique_ptr<Dictionary> m_dictionary;
};

struct MaterialNode : Node {
    MaterialNode(NodeID id) noexcept : Node(id) {}
};

struct RootMaterialNode : MaterialNode {
    RootMaterialNode(const RootMaterialNode&) = delete;
    RootMaterialNode& operator=(const RootMaterialNode&) = delete;

    auto AddMaterialClassNode() -> ClassMaterialNodePtr;

    auto GetChildren() const -> NodeArray override;

    json ToJson() const override;

  private:
    friend struct NodeAccess;

    RootMaterialNode(NodeID id) noexcept : MaterialNode(id) {}

    std::vector<UniqueNode> m_nodes;
};

struct ClassMaterialNode : MaterialNode {
    ClassMaterialNode(const ClassMaterialNode&) = delete;
    ClassMaterialNode& operator=(const ClassMaterialNode&) = delete;

    auto AddMaterialInstanceNode() -> InstanceMaterialNodePtr;

    auto GetChildren() const -> NodeArray override;

    json ToJson() const override;

  private:
    friend struct NodeAccess;

    ClassMaterialNode(NodeID id) noexcept : MaterialNode(id){};

    std::vector<UniqueNode> m_nodes;
};

struct InstanceMaterialNode : MaterialNode {
    InstanceMaterialNode(const InstanceMaterialNode&) = delete;
    InstanceMaterialNode& operator=(const InstanceMaterialNode&) = delete;

    auto GetChildren() const -> NodeArray override { return {}; }

    json ToJson() const override;

  private:
    friend struct NodeAccess;

    InstanceMaterialNode(NodeID id) noexcept : MaterialNode(id) {}

    void AddMeshInstancePtr(InstanceGeoNodePtr mesh_instance) { m_mesh_instances.push_back(mesh_instance); }

    std::vector<InstanceGeoNodePtr> m_mesh_instances;
};

struct GeoNode : Node {
    GeoNode(NodeID id) noexcept : Node(id) {}

  private:
    virtual void UpdateInstances() noexcept = 0;

    friend struct NodeAccess;
};

struct InstanceGeoNode : GeoNode {
    InstanceGeoNode(const InstanceGeoNode&) = delete;
    InstanceGeoNode& operator=(const InstanceGeoNode&) = delete;

    auto GetChildren() const -> NodeArray override { return {}; }

    json ToJson() const override;

  private:
    friend struct NodeAccess;

    virtual void UpdateInstances() noexcept override;

    InstanceGeoNode(NodeID id, InstanceMaterialNodePtr material_instance) noexcept;

    InstanceMaterialNodePtr m_material_instance;
};

struct InternalGeoNode : GeoNode {
    auto AddTranslateNode(glm::vec3 amount) -> TranslateGeoNodePtr;
    auto AddRotateNode(glm::vec3 axis, Radians angle) -> RotateGeoNodePtr;
    auto AddScaleNode(float factor) -> ScaleGeoNodePtr;
    auto AddInstanceNode(InstanceMaterialNodePtr material_instance) -> InstanceGeoNodePtr;

    auto GetChildren() const -> NodeArray override;

  protected:
    friend struct NodeAccess;

    InternalGeoNode(NodeID id) noexcept : GeoNode(id) {}

    std::vector<UniqueNode> m_nodes;
};

struct RootGeoNode : InternalGeoNode {
    RootGeoNode(const RootGeoNode&) = delete;
    RootGeoNode& operator=(const RootGeoNode&) = delete;

    json ToJson() const override;

  private:
    friend struct NodeAccess;

    RootGeoNode(NodeID id) noexcept : InternalGeoNode(id) {}

    virtual void UpdateInstances() noexcept override;
};

struct TranslateGeoNode : InternalGeoNode {
    TranslateGeoNode(const TranslateGeoNode&) = delete;
    TranslateGeoNode& operator=(const TranslateGeoNode&) = delete;

    json ToJson() const override;

  private:
    friend struct NodeAccess;

    TranslateGeoNode(NodeID id, glm::vec3 distance) noexcept : InternalGeoNode(id), m_distance(distance) {}

    virtual void UpdateInstances() noexcept override;

    glm::vec3 m_distance;
};

struct RotateGeoNode : InternalGeoNode {
    RotateGeoNode(const RotateGeoNode&) = delete;
    RotateGeoNode& operator=(const RotateGeoNode&) = delete;

    json ToJson() const override;

  private:
    friend struct NodeAccess;

    RotateGeoNode(NodeID id, glm::vec3 axis, Radians angle) noexcept : InternalGeoNode(id), m_axis(axis), m_angle(angle)
    {}

    virtual void UpdateInstances() noexcept override;

    glm::vec3 m_axis;
    Radians   m_angle;
};

struct ScaleGeoNode : InternalGeoNode {
    ScaleGeoNode(const ScaleGeoNode&) = delete;
    ScaleGeoNode& operator=(const ScaleGeoNode&) = delete;

    json ToJson() const override;

  private:
    friend struct NodeAccess;

    ScaleGeoNode(NodeID id, float factor) noexcept : InternalGeoNode(id), m_factor(factor) {}

    virtual void UpdateInstances() noexcept override;

    float m_factor;
};

struct Scene {
    Scene();

    Scene(const Scene&) = delete;
    Scene operator=(const Scene&) = delete;

    auto GetGeometryRoot() noexcept -> RootGeoNodePtr;
    auto GetMaterialRoot() noexcept -> RootMaterialNodePtr;

    void UpdateInstances() noexcept;

    json ToJson() const;

  private:
    UniqueNode m_materials;
    UniqueNode m_geometry;
};
