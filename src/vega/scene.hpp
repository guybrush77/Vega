#pragma once

#include "utils/math.hpp"
#include "vertex.hpp"

BEGIN_DISABLE_WARNINGS

#include <glm/gtx/transform.hpp>
#include <glm/matrix.hpp>

END_DISABLE_WARNINGS

#include <nlohmann/json.hpp>

#include <any>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

struct ID;

struct MeshVertices;
struct MeshIndices;
struct Mesh;
struct MeshManager;

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

using MeshVerticesPtr = MeshVertices*;
using MeshIndicesPtr  = MeshIndices*;
using UniqueMesh      = std::unique_ptr<Mesh>;
using MeshPtr         = Mesh*;

using Key        = std::string;
using Value      = std::variant<int, float, std::string>;
using Dictionary = std::unordered_map<Key, Value>;

using UniqueNode = std::unique_ptr<Node>;
using NodePtr    = Node*;
using NodeRef    = Node&;
using NodeArray  = std::vector<NodePtr>;

using MaterialNodePtr     = MaterialNode*;
using RootMaterialNodePtr = RootMaterialNode*;
using MaterialClassPtr    = ClassMaterialNode*;
using MaterialInstancePtr = InstanceMaterialNode*;

using GeoNodePtr       = GeoNode*;
using RootGeoNodePtr   = RootGeoNode*;
using TranslateNodePtr = TranslateGeoNode*;
using RotateNodePtr    = RotateGeoNode*;
using ScaleNodePtr     = ScaleGeoNode*;
using MeshInstancePtr  = InstanceGeoNode*;

using UniqueMeshManager = std::unique_ptr<MeshManager>;

using ScenePtr = Scene*;

using json = nlohmann::json;

struct ID {
    int value;

    constexpr bool operator==(const ID&) const noexcept = default;
    struct Hash {
        constexpr size_t operator()(ID id) const noexcept { return id.value; }
    };
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

struct MeshVertices final {
    template <VertexType T>
    MeshVertices(std::vector<T> vertices) : m_vertices(std::move(vertices))
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

    MeshVertices(const MeshVertices&) = delete;
    MeshVertices& operator=(const MeshVertices&) = delete;

    MeshVertices(MeshVertices&&) = default;
    MeshVertices& operator=(MeshVertices&&) = default;

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

struct MeshIndices final {
    template <IndexType T>
    MeshIndices(std::vector<T> indices) : m_indices(indices)
    {
        using etna::narrow_cast;

        auto& indices_ref = std::any_cast<std::vector<T>&>(m_indices);

        indices_ref.shrink_to_fit();

        m_type       = typeid(indices_ref[0]).name();
        m_index_size = narrow_cast<uint32_t>(sizeof(indices_ref[0]));
        m_count      = narrow_cast<uint32_t>(indices_ref.size());
        m_size       = narrow_cast<uint32_t>(m_index_size * m_count);
        m_data       = static_cast<const void*>(indices_ref.data());
    }

    MeshIndices(const MeshIndices&) = delete;
    MeshIndices& operator=(const MeshIndices&) = delete;

    MeshIndices(MeshIndices&&) = default;
    MeshIndices& operator=(MeshIndices&&) = default;

    auto Type() const noexcept { return m_type; }
    auto IndexSize() const noexcept { return m_index_size; }
    auto Count() const noexcept { return m_count; }
    auto Size() const noexcept { return m_size; }
    auto Data() const noexcept { return m_data; }

  private:
    std::any    m_indices;
    const char* m_type       = nullptr;
    uint32_t    m_index_size = 0;
    uint32_t    m_count      = 0;
    uint32_t    m_size       = 0;
    const void* m_data       = nullptr;
};

struct Mesh final : PropertyDictionary {
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    Mesh(Mesh&&)  = default;
    Mesh& operator=(Mesh&&) = default;

    auto GetID() const noexcept -> ID { return m_id; }
    auto BoundingBox() const noexcept -> const AABB& { return m_aabb; }
    auto Vertices() const noexcept -> const MeshVertices& { return m_vertices; }
    auto Indices() const noexcept -> const MeshIndices& { return m_indices; }

    json ToJson() const;

  private:
    friend struct MeshManager;

    Mesh(ID id, AABB aabb, MeshVertices mesh_vertices, MeshIndices mesh_indices) noexcept
        : m_id(id), m_aabb(aabb), m_vertices(std::move(mesh_vertices)), m_indices(std::move(mesh_indices))
    {}

    ID           m_id{};
    AABB         m_aabb{};
    MeshVertices m_vertices;
    MeshIndices  m_indices;
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

    auto AddMaterialClassNode() -> MaterialClassPtr;

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

    auto AddMaterialInstanceNode() -> MaterialInstancePtr;

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

    auto GetChildren() const -> NodeArray override { return m_nodes; }

    json ToJson() const override;

  private:
    friend struct NodeAccess;

    InstanceMaterialNode(NodeID id) noexcept : MaterialNode(id) {}

    void AddMeshInstancePtr(MeshInstancePtr mesh_instance);

    NodeArray m_nodes;
};

struct GeoNode : Node {
    GeoNode(NodeID id) noexcept : Node(id) {}

  private:
    virtual void ApplyTransform(const glm::mat4& matrix) noexcept = 0;

    friend struct NodeAccess;
};

struct InternalGeoNode : GeoNode {
    auto AddTranslateNode(glm::vec3 amount) -> TranslateNodePtr;
    auto AddRotateNode(glm::vec3 axis, Radians angle) -> RotateNodePtr;
    auto AddScaleNode(float factor) -> ScaleNodePtr;
    auto AddInstanceNode(MeshPtr mesh, MaterialInstancePtr material) -> MeshInstancePtr;

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

    virtual void ApplyTransform(const glm::mat4& matrix) noexcept override;
};

struct TranslateGeoNode final : InternalGeoNode {
    TranslateGeoNode(const TranslateGeoNode&) = delete;
    TranslateGeoNode& operator=(const TranslateGeoNode&) = delete;

    json ToJson() const override;

  private:
    friend struct NodeAccess;

    TranslateGeoNode(NodeID id, glm::vec3 distance) noexcept : InternalGeoNode(id), m_distance(distance) {}

    virtual void ApplyTransform(const glm::mat4& matrix) noexcept override;

    glm::vec3 m_distance;
};

struct RotateGeoNode final : InternalGeoNode {
    RotateGeoNode(const RotateGeoNode&) = delete;
    RotateGeoNode& operator=(const RotateGeoNode&) = delete;

    json ToJson() const override;

    auto Axis() const noexcept { return m_axis; }
    auto Angle() const noexcept { return m_angle; }

    void SetAxis(const glm::vec3& axis) noexcept { m_axis = axis; }
    void SetAngle(Radians angle) noexcept { m_angle = angle; }

  private:
    friend struct NodeAccess;

    RotateGeoNode(NodeID id, glm::vec3 axis, Radians angle) noexcept : InternalGeoNode(id), m_axis(axis), m_angle(angle)
    {}

    virtual void ApplyTransform(const glm::mat4& matrix) noexcept override;

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

    virtual void ApplyTransform(const glm::mat4& matrix) noexcept override;

    float m_factor;
};

struct InstanceGeoNode final : GeoNode {
    InstanceGeoNode(const InstanceGeoNode&) = delete;
    InstanceGeoNode& operator=(const InstanceGeoNode&) = delete;

    auto GetChildren() const -> NodeArray override { return NodeArray{}; }

    auto GetMeshPtr() const noexcept { return m_mesh; }

    auto GetMaterialInstancePtr() const noexcept { return m_material_instance; }

    auto GetTransformPtr() const noexcept -> const glm::mat4* { return &m_transform; }

    json ToJson() const override;

  private:
    friend struct NodeAccess;

    virtual void ApplyTransform(const glm::mat4& matrix) noexcept override;

    InstanceGeoNode(NodeID id, MeshPtr mesh, MaterialInstancePtr material) noexcept;

    MeshPtr             m_mesh              = nullptr;
    MaterialInstancePtr m_material_instance = nullptr;
    glm::mat4           m_transform         = glm::mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
};

struct DrawRecord final {
    MeshPtr          mesh;
    const glm::mat4* transform;
};

using DrawList = std::vector<DrawRecord>;

struct Scene {
    Scene();

    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    Scene(Scene&&) = default;
    Scene& operator=(Scene&&) = default;

    ~Scene() noexcept;

    auto GetGeometryRootPtr() noexcept -> RootGeoNodePtr;
    auto GetMaterialRootPtr() noexcept -> RootMaterialNodePtr;

    auto CreateMesh(AABB aabb, MeshVertices mesh_vertices, MeshIndices mesh_indices) -> MeshPtr;

    auto GetDrawList() const -> DrawList;

    json ToJson() const;

  private:
    UniqueNode        m_materials;
    UniqueNode        m_geometry;
    UniqueMeshManager m_mesh_manager;
};
