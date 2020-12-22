#pragma once

#include "utils/math.hpp"
#include "utils/misc.hpp"
#include "vertex.hpp"

BEGIN_DISABLE_WARNINGS

#include <glm/gtx/transform.hpp>
#include <glm/matrix.hpp>

END_DISABLE_WARNINGS

#include <nlohmann/json.hpp>

#include <any>
#include <memory>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

class Material;
class MaterialInstance;
class Mesh;
class MaterialManager;
class MeshManager;
class MeshNode;
class Node;
class RootNode;
class RotateNode;
class ScaleNode;
class Scene;
class TranslateNode;

using MaterialPtr         = Material*;
using MaterialInstancePtr = MaterialInstance*;
using MeshPtr             = Mesh*;
using MeshNodePtr         = MeshNode*;
using NodePtr             = Node*;
using RootNodePtr         = RootNode*;
using RotateNodePtr       = RotateNode*;
using ScaleNodePtr        = ScaleNode*;
using ScenePtr            = Scene*;
using TranslateNodePtr    = TranslateNode*;

using UniqueMaterial        = std::unique_ptr<Material>;
using UniqueMesh            = std::unique_ptr<Mesh>;
using UniqueMaterialManager = std::unique_ptr<MaterialManager>;
using UniqueMeshManager     = std::unique_ptr<MeshManager>;
using UniqueNode            = std::unique_ptr<Node>;

using MaterialArray = std::vector<MaterialPtr>;
using NodeArray     = std::vector<NodePtr>;

struct Metadata;

using MetadataRef = const Metadata&;

// TODO

/*
using Key        = std::string;
using Value      = std::variant<int, float, std::string>;
using Dictionary = std::unordered_map<Key, Value>;
*/

using json = nlohmann::json;

enum class ValueType { Null, Float, Vec3 };

inline const char* to_string(ValueType value_type) noexcept
{
    switch (value_type) {
    case ValueType::Null: return "Null";
    case ValueType::Float: return "Float";
    case ValueType::Vec3: return "Vec3";
    default: throw_runtime_error("ValueType: Bad enum value");
    };
}

struct Field final {
    using IsEditable = bool;

    const char* name{};
    const char* label{};
    const char* description{};
    ValueType   value_type{};
    IsEditable  is_editable{};
};

struct Metadata final {
    const char* object_class{};
    const char* object_label{};
    const char* object_description{};

    std::vector<Field> fields;
};

struct ID final {
    std::uint64_t value = 0;

    constexpr ID() noexcept = default;
    constexpr ID(std::uint64_t value) noexcept : value(value) {}

    constexpr operator std::uint64_t() const noexcept { return value; }

    constexpr bool operator==(const ID&) const noexcept = default;

    struct Hash {
        constexpr size_t operator()(ID id) const noexcept { return static_cast<size_t>(id.value); }
    };
};

/*
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
*/

class ValueRef final {
  public:
    ValueRef() noexcept = default;

    explicit ValueRef(float* value) noexcept : m_type(ValueType::Float), value(value) {}
    explicit ValueRef(glm::vec3* value) noexcept : m_type(ValueType::Vec3), value(value) {}

    operator float&() const
    {
        throw_runtime_error_if(m_type != ValueType::Float, "Cannot convert {} to float", to_string(m_type));
        return *static_cast<float*>(value);
    }

    operator glm::vec3 &() const
    {
        throw_runtime_error_if(m_type != ValueType::Float, "Cannot convert {} to glm::vec3", to_string(m_type));
        return *static_cast<glm::vec3*>(value);
    }

  private:
    using ValuePtr = void*;

    ValueType m_type = ValueType::Null;
    ValuePtr  value  = nullptr;
};

class Object {
  public:
    virtual ~Object() noexcept = default;

    auto GetID() const noexcept { return m_id; }

    virtual bool HasProperties() const noexcept { return false; }
    virtual auto GetMetadata() const -> MetadataRef = 0;
    virtual auto ToJson() const -> json             = 0;

    virtual auto GetField(std::string_view field_name) -> ValueRef = 0;

  protected:
    friend struct ObjectAccess;

    Object(ID id) noexcept : m_id(id) {}

    ID m_id;
};

class MeshVertices final {
  public:
    template <VertexType T, typename A>
    MeshVertices(std::vector<T, A> vertices) : m_vertices(std::move(vertices))
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

class MeshIndices final {
  public:
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

class Mesh final {
  public:
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    Mesh(Mesh&&)  = default;
    Mesh& operator=(Mesh&&) = default;

    auto GetID() const noexcept -> ID { return m_id; }
    auto GetAxisAlignedBoundingBox() const noexcept -> const AABB& { return m_aabb; }
    auto Vertices() const noexcept -> const MeshVertices& { return m_vertices; }
    auto Indices() const noexcept -> const MeshIndices& { return m_indices; }

    json ToJson() const;

  private:
    friend class MeshManager;

    Mesh(ID id, AABB aabb, MeshVertices mesh_vertices, MeshIndices mesh_indices) noexcept
        : m_id(id), m_aabb(aabb), m_vertices(std::move(mesh_vertices)), m_indices(std::move(mesh_indices))
    {}

    ID           m_id{};
    AABB         m_aabb{};
    MeshVertices m_vertices;
    MeshIndices  m_indices;
};

class Material final : public Object {
  public:
    Material(const Material&) = delete;
    Material& operator=(const Material&) = delete;

    Material(Material&&) = default;
    Material& operator=(Material&&) = default;

    auto GetMetadata() const -> MetadataRef override { return metadata; }

    auto GetField(std::string_view) -> ValueRef override { return {}; }

    auto AddMaterialInstance() -> MaterialInstancePtr;

    json ToJson() const override;

  private:
    friend class MaterialManager;
    friend struct ObjectAccess;

    Material(ID id) noexcept : Object(id){};

    inline static Metadata metadata = { "root.material", "Material", nullptr };

    std::vector<UniqueNode> m_children;
};

/*
struct InstanceMaterialNode final : MaterialNode {
    InstanceMaterialNode(const InstanceMaterialNode&) = delete;
    InstanceMaterialNode& operator=(const InstanceMaterialNode&) = delete;

    auto GetMetaData() const -> MetaDataRef override { return metadata; }

    auto GetChildren() const -> NodeArray override { return m_children; }

    json ToJson() const override;

  private:
    friend struct ObjectAccess;

    InstanceMaterialNode(NodeID id) noexcept : MaterialNode(id) {}

    void AddInstantiateObjectNodePtr(InstantiateObjectNodePtr object_instance);

    inline static const char*    node_class = "instance.material";
    inline static const Metadata metadata   = { "Material Instance" };

    NodeArray m_children;
};
*/

class Node : public Object {
  public:
    Node(ID id) noexcept : Object(id) {}

    virtual auto GetChildren() const -> NodeArray = 0;

  protected:
    friend struct ObjectAccess;

    virtual void ApplyTransform(const glm::mat4& matrix) noexcept = 0;
};

class InternalNode : public Node {
  public:
    auto AddTranslateNode(glm::vec3 amount) -> TranslateNodePtr;
    auto AddRotateNode(glm::vec3 axis, Radians angle) -> RotateNodePtr;
    auto AddScaleNode(float factor) -> ScaleNodePtr;
    auto AddMeshNode(MeshPtr mesh, MaterialInstancePtr material) -> MeshNodePtr;

    auto GetChildren() const -> NodeArray override;

  protected:
    friend struct ObjectAccess;

    InternalNode(ID id) noexcept : Node(id) {}

    std::vector<UniqueNode> m_children;
};

class RootNode final : public InternalNode {
  public:
    RootNode(const RootNode&) = delete;
    RootNode& operator=(const RootNode&) = delete;

    auto GetMetadata() const -> MetadataRef override { return metadata; }

    auto GetField(std::string_view) -> ValueRef override { return {}; }

    json ToJson() const override;

  private:
    friend struct ObjectAccess;

    inline static Metadata metadata = { "root.node", "Root", nullptr };

    RootNode(ID id) noexcept : InternalNode(id) {}

    virtual void ApplyTransform(const glm::mat4& matrix) noexcept override;
};

class TranslateNode final : public InternalNode {
    TranslateNode(const TranslateNode&) = delete;
    TranslateNode& operator=(const TranslateNode&) = delete;

    auto GetMetadata() const -> MetadataRef override { return metadata; }

    auto GetField(std::string_view field_name) -> ValueRef override
    {
        return field_name == "translate.amount" ? ValueRef(&m_amount) : ValueRef();
    }

    json ToJson() const override;

  private:
    friend struct ObjectAccess;

    inline static Metadata metadata = {
        "translate.node",
        "Translate",
        nullptr,
        { Field{ "translate.amount", "Amount", nullptr, ValueType::Vec3, Field::IsEditable{ true } } }
    };

    TranslateNode(ID id, glm::vec3 amount) noexcept : InternalNode(id), m_amount(amount) {}

    virtual void ApplyTransform(const glm::mat4& matrix) noexcept override;

    glm::vec3 m_amount;
};

class RotateNode final : public InternalNode {
  public:
    RotateNode(const RotateNode&) = delete;
    RotateNode& operator=(const RotateNode&) = delete;

    auto GetMetadata() const -> MetadataRef override { return metadata; }

    auto GetField(std::string_view field_name) -> ValueRef override
    {
        return field_name == "rotate.axis" ? ValueRef(&m_axis)
                                           : field_name == "rotate.angle" ? ValueRef(&m_angle.value) : ValueRef();
    }

    json ToJson() const override;

  private:
    friend struct ObjectAccess;

    inline static Metadata metadata = {
        "rotate.node",
        "Rotate",
        nullptr,
        { Field{ "rotate.axis", "Axis", nullptr, ValueType::Vec3, Field::IsEditable{ true } },
          Field{ "rotate.angle", "Angle", nullptr, ValueType::Float, Field::IsEditable{ true } } }
    };

    RotateNode(ID id, glm::vec3 axis, Radians angle) noexcept : InternalNode(id), m_axis(axis), m_angle(angle) {}

    virtual void ApplyTransform(const glm::mat4& matrix) noexcept override;

    glm::vec3 m_axis;
    Radians   m_angle;
};

class ScaleNode final : public InternalNode {
  public:
    ScaleNode(const ScaleNode&) = delete;
    ScaleNode& operator=(const ScaleNode&) = delete;

    auto GetMetadata() const -> MetadataRef override { return metadata; }

    auto GetField(std::string_view field_name) -> ValueRef override
    {
        return field_name == "scale.factor" ? ValueRef(&m_factor) : ValueRef();
    }

    json ToJson() const override;

  private:
    friend struct ObjectAccess;

    inline static Metadata metadata = {
        "scale.node",
        "Scale",
        nullptr,
        { Field{ "scale.factor", "Factor", nullptr, ValueType::Float, Field::IsEditable{ true } } }
    };

    ScaleNode(ID id, float factor) noexcept : InternalNode(id), m_factor(factor) {}

    virtual void ApplyTransform(const glm::mat4& matrix) noexcept override;

    float m_factor;
};

class MeshNode final : public Node {
  public:
    MeshNode(const MeshNode&) = delete;
    MeshNode& operator=(const MeshNode&) = delete;

    auto GetMetadata() const -> MetadataRef override { return metadata; }
    auto GetField(std::string_view) -> ValueRef override { return {}; } // TODO
    auto GetChildren() const -> NodeArray override { return NodeArray{}; }

    //    auto GetMeshPtr() const noexcept { return m_mesh; }

    auto GetMaterialInstancePtr() const noexcept { return m_material_instance; }

    auto GetTransformPtr() const noexcept -> const glm::mat4* { return &m_transform; }

    json ToJson() const override;

  private:
    friend struct ObjectAccess;

    inline static Metadata metadata{}; // TODO

    MeshNode(ID id, MeshPtr mesh, MaterialInstancePtr material) noexcept;

    virtual void ApplyTransform(const glm::mat4& matrix) noexcept override;

    MeshPtr             m_mesh              = nullptr;
    MaterialInstancePtr m_material_instance = nullptr;
    glm::mat4           m_transform         = glm::mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
};

struct DrawRecord final {
    MeshPtr          mesh;
    const glm::mat4* transform;
};

using DrawList = std::vector<DrawRecord>;

class Scene {
  public:
    Scene();

    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    Scene(Scene&&) = default;
    Scene& operator=(Scene&&) = default;

    ~Scene() noexcept;

    auto GetRootNodePtr() noexcept -> RootNodePtr;

    auto CreateMaterial() -> MaterialPtr;

    auto CreateMesh(AABB aabb, MeshVertices mesh_vertices, MeshIndices mesh_indices) -> MeshPtr;

    auto ComputeDrawList() const -> DrawList;

    auto ComputeAxisAlignedBoundingBox() const -> AABB;

    json ToJson() const;

  private:
    UniqueNode            m_root_node;
    UniqueMaterialManager m_material_manager;
    UniqueMeshManager     m_mesh_manager;
};
