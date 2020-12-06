#pragma once

#include "utils/math.hpp"
#include "vertex.hpp"

#include <nlohmann/json.hpp>

#include <any>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

struct Vertices;
struct Indices;
struct Mesh;

struct Node;

struct MaterialNode;
struct RootMaterialNode;
struct ClassMaterialNode;
struct InstanceMaterialNode;

struct GeoNode;
struct RootGeoNode;
struct TranslateGeoNode;
struct RotateGeoNode;
struct ScaleGeoNode;
struct InstanceGeoNode;

struct Scene;

using VerticesPtr = Vertices*;
using IndicesPtr  = Indices*;
using UniqueMesh  = std::unique_ptr<Mesh>;
using MeshPtr     = Mesh*;

using Key        = std::string;
using Value      = std::variant<int, float, std::string>;
using Dictionary = std::unordered_map<Key, Value>;

using UniqueNode = std::unique_ptr<Node>;
using NodePtr    = Node*;
using NodeRef    = Node&;
using NodeArray  = std::vector<NodePtr>;

using MaterialNodePtr         = MaterialNode*;
using RootMaterialNodePtr     = RootMaterialNode*;
using ClassMaterialNodePtr    = ClassMaterialNode*;
using InstanceMaterialNodePtr = InstanceMaterialNode*;

using GeoNodePtr          = GeoNode*;
using RootGeoNodePtr      = RootGeoNode*;
using TranslateGeoNodePtr = TranslateGeoNode*;
using RotateGeoNodePtr    = RotateGeoNode*;
using ScaleGeoNodePtr     = ScaleGeoNode*;
using InstanceGeoNodePtr  = InstanceGeoNode*;

using ScenePtr = Scene*;

using json = nlohmann::json;

struct ID {
    int value;
};

struct PropertyDictionary {
    auto GetProperty(const Key& key) const noexcept -> const Value&;
    bool HasProperty(const Key& key) const noexcept;
    bool HasProperties() const noexcept;
    void SetProperty(Key key, int value);
    void SetProperty(Key key, float value);
    void SetProperty(Key key, std::string value);
    void RemoveProperty(Key key);

    json ToJson() const;

  private:
    inline static const Value nullvalue;

    void SetPropertyPrivate(Key key, Value value);

    std::unique_ptr<Dictionary> m_dictionary;
};

struct Vertices final {
    template <VertexType T>
    Vertices(std::vector<T> vertices) : m_vertices(std::move(vertices))
    {
        using etna::narrow_cast;

        auto& vertices_ref = std::any_cast<std::vector<T>&>(m_vertices);

        vertices_ref.shrink_to_fit();

        m_type        = typeid(vertices_ref[0]).name();
        m_vertex_size = narrow_cast<uint32_t>(sizeof(vertices_ref[0]));
        m_count       = narrow_cast<uint32_t>(vertices_ref.size());
        m_size        = narrow_cast<uint32_t>(m_vertex_size * m_count);
        m_data        = static_cast<const void*>(vertices_ref.data());
    }

    Vertices(const Vertices&) = delete;
    Vertices& operator=(const Vertices&) = delete;

    Vertices(Vertices&&) = default;
    Vertices& operator=(Vertices&&) = default;

    auto Type() const noexcept { return m_type; }
    auto VertexSize() const noexcept { return m_vertex_size; }
    auto Count() const noexcept { return m_count; }
    auto Size() const noexcept { return m_size; }
    auto Data() const noexcept { return m_data; }

  private:
    std::any    m_vertices;
    std::string m_type;
    uint32_t    m_vertex_size = 0;
    uint32_t    m_count       = 0;
    uint32_t    m_size        = 0;
    const void* m_data        = nullptr;
};

struct Indices final {
    Indices(std::vector<uint16_t> indices) : m_indices(indices)
    {
        using etna::narrow_cast;

        auto& indices_ref = std::get<decltype(indices)>(m_indices);

        indices_ref.shrink_to_fit();

        m_type       = typeid(indices_ref[0]).name();
        m_index_size = narrow_cast<uint32_t>(sizeof(indices_ref[0]));
        m_count      = narrow_cast<uint32_t>(indices_ref.size());
        m_size       = narrow_cast<uint32_t>(m_index_size * m_count);
        m_data       = static_cast<const void*>(indices_ref.data());
    }

    Indices(std::vector<uint32_t> indices) : m_indices(indices)
    {
        using etna::narrow_cast;

        auto& indices_ref = std::get<decltype(indices)>(m_indices);

        indices_ref.shrink_to_fit();

        m_type       = typeid(indices_ref[0]).name();
        m_index_size = narrow_cast<uint32_t>(sizeof(indices_ref[0]));
        m_count      = narrow_cast<uint32_t>(indices_ref.size());
        m_size       = narrow_cast<uint32_t>(m_index_size * m_count);
        m_data       = static_cast<const void*>(indices_ref.data());
    }

    Indices(const Indices&) = delete;
    Indices& operator=(const Indices&) = delete;

    Indices(Indices&&) = default;
    Indices& operator=(Indices&&) = default;

    auto Type() const noexcept { return m_type; }
    auto IndexSize() const noexcept { return m_index_size; }
    auto Count() const noexcept { return m_count; }
    auto Size() const noexcept { return m_size; }
    auto Data() const noexcept { return m_data; }

  private:
    std::variant<std::vector<uint16_t>, std::vector<uint32_t>> m_indices;

    const char* m_type       = nullptr;
    uint32_t    m_index_size = 0;
    uint32_t    m_count      = 0;
    uint32_t    m_size       = 0;
    const void* m_data       = nullptr;
};

struct MeshID final : ID {
    constexpr MeshID() noexcept : ID({}) {}
    constexpr MeshID(ID id) noexcept : ID(id) {}
};

struct Mesh final : PropertyDictionary {
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    Mesh(Mesh&&)  = default;
    Mesh& operator=(Mesh&&) = default;

    auto ID() const noexcept { return m_id; }

    json ToJson() const;

  private:
    friend struct MeshManager;

    Mesh(MeshID id, AABB aabb, Vertices vertices, Indices indices) noexcept
        : m_id(id), m_aabb(aabb), m_vertices(std::move(vertices)), m_indices(std::move(indices))
    {}

    MeshID   m_id{};
    AABB     m_aabb{};
    Vertices m_vertices;
    Indices  m_indices;
};

struct MeshManager {
    MeshPtr CreateMesh(AABB aabb, Vertices vertices, Indices indices);

    json ToJson() const;

  private:
    std::unordered_map<MeshPtr, UniqueMesh> m_meshes;
};

struct NodeID final : ID {
    constexpr NodeID() noexcept : ID({}) {}
    constexpr NodeID(ID id) noexcept : ID(id) {}
};

struct Node : PropertyDictionary {
    virtual ~Node() noexcept = default;

    auto ID() const noexcept { return m_id; }

    virtual auto GetChildren() const -> NodeArray = 0;
    virtual auto ToJson() const -> json           = 0;

  protected:
    friend struct NodeAccess;

    Node(NodeID id) noexcept : m_id(id) {}

    NodeID m_id;
};

struct MaterialNode : Node {
    MaterialNode(NodeID id) noexcept : Node(id) {}
};

struct RootMaterialNode final : MaterialNode {
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

struct ClassMaterialNode final : MaterialNode {
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

struct InstanceMaterialNode final : MaterialNode {
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

struct InstanceGeoNode final : GeoNode {
    InstanceGeoNode(const InstanceGeoNode&) = delete;
    InstanceGeoNode& operator=(const InstanceGeoNode&) = delete;

    auto GetChildren() const -> NodeArray override { return {}; }

    json ToJson() const override;

  private:
    friend struct NodeAccess;

    virtual void UpdateInstances() noexcept override;

    InstanceGeoNode(NodeID id, MeshPtr mesh, InstanceMaterialNodePtr material) noexcept;

    MeshPtr                 m_mesh_instance     = nullptr;
    InstanceMaterialNodePtr m_material_instance = nullptr;
};

struct InternalGeoNode : GeoNode {
    auto AddTranslateNode(glm::vec3 amount) -> TranslateGeoNodePtr;
    auto AddRotateNode(glm::vec3 axis, Radians angle) -> RotateGeoNodePtr;
    auto AddScaleNode(float factor) -> ScaleGeoNodePtr;
    auto AddInstanceNode(MeshPtr mesh, InstanceMaterialNodePtr material) -> InstanceGeoNodePtr;

    auto GetChildren() const -> NodeArray override;

  protected:
    friend struct NodeAccess;

    InternalGeoNode(NodeID id) noexcept : GeoNode(id) {}

    std::vector<UniqueNode> m_nodes;
};

struct RootGeoNode final : InternalGeoNode {
    RootGeoNode(const RootGeoNode&) = delete;
    RootGeoNode& operator=(const RootGeoNode&) = delete;

    json ToJson() const override;

  private:
    friend struct NodeAccess;

    RootGeoNode(NodeID id) noexcept : InternalGeoNode(id) {}

    virtual void UpdateInstances() noexcept override;
};

struct TranslateGeoNode final : InternalGeoNode {
    TranslateGeoNode(const TranslateGeoNode&) = delete;
    TranslateGeoNode& operator=(const TranslateGeoNode&) = delete;

    json ToJson() const override;

  private:
    friend struct NodeAccess;

    TranslateGeoNode(NodeID id, glm::vec3 distance) noexcept : InternalGeoNode(id), m_distance(distance) {}

    virtual void UpdateInstances() noexcept override;

    glm::vec3 m_distance;
};

struct RotateGeoNode final : InternalGeoNode {
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

struct ScaleGeoNode final : InternalGeoNode {
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

    MeshPtr CreateMesh(AABB aabb, Vertices vertices, Indices indices);

    void UpdateInstances() noexcept;

    json ToJson() const;

  private:
    UniqueNode  m_materials;
    UniqueNode  m_geometry;
    MeshManager m_mesh_manager;
};
