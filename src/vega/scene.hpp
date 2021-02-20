#pragma once

#include "platform.hpp"
#include "utils/cast.hpp"
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

class GroupNode;
class InstanceNode;
class Material;
class Mesh;
class MeshManager;
class Node;
class Object;
class RootNode;
class RotateNode;
class ScaleNode;
class Scene;
class Shader;
class ShaderManager;
class TranslateNode;

using GroupNodePtr     = GroupNode*;
using InstanceNodePtr  = InstanceNode*;
using MaterialPtr      = Material*;
using MeshPtr          = Mesh*;
using NodePtr          = Node*;
using ObjectPtr        = Object*;
using RootNodePtr      = RootNode*;
using RotateNodePtr    = RotateNode*;
using ScaleNodePtr     = ScaleNode*;
using ScenePtr         = Scene*;
using ShaderPtr        = Shader*;
using TranslateNodePtr = TranslateNode*;

using UniqueMaterial      = std::unique_ptr<Material>;
using UniqueMesh          = std::unique_ptr<Mesh>;
using UniqueMeshManager   = std::unique_ptr<MeshManager>;
using UniqueNode          = std::unique_ptr<Node>;
using UniqueShader        = std::unique_ptr<Shader>;
using UniqueShaderManager = std::unique_ptr<ShaderManager>;

using InstanceNodePtrArray = std::vector<InstanceNodePtr>;
using MaterialPtrArray     = std::vector<MaterialPtr>;
using MeshPtrArray         = std::vector<MeshPtr>;
using NodePtrArray         = std::vector<NodePtr>;
using ObjectPtrArray       = std::vector<ObjectPtr>;
using ShaderPtrArray       = std::vector<ShaderPtr>;

using MaterialRefArray = std::vector<std::reference_wrapper<Material>>;
using MeshRefArray     = std::vector<std::reference_wrapper<Mesh>>;
using NodeRefArray     = std::vector<std::reference_wrapper<Node>>;
using ObjectRefArray   = std::vector<std::reference_wrapper<Object>>;
using ShaderRefArray   = std::vector<std::reference_wrapper<Shader>>;

struct Metadata;

using MetadataRef = const Metadata&;

using Key           = std::string;
using Value         = std::variant<int, float, std::string>;
using Dictionary    = std::unordered_map<Key, Value>;
using DictionaryRef = const Dictionary&;

using json = nlohmann::json;

struct ID final {
    int value = 0;

    constexpr ID() noexcept = default;
    constexpr ID(int value) noexcept : value(value) {}

    constexpr operator int() const noexcept { return value; }

    constexpr bool operator==(const ID&) const noexcept = default;

    struct Hash {
        constexpr size_t operator()(ID id) const noexcept { return static_cast<size_t>(id.value); }
    };
};

enum class ValueType { Null, Float, Float3, Int, Reference, String };

std::string to_string(ValueType value_type);

class ValueRef final {
  public:
    ValueRef() noexcept = default;

    explicit ValueRef(float* value) noexcept : m_type(ValueType::Float), value(value) {}
    explicit ValueRef(Float3* value) noexcept : m_type(ValueType::Float3), value(value) {}
    explicit ValueRef(int* value) noexcept : m_type(ValueType::Int), value(value) {}
    explicit ValueRef(Object* value) noexcept : m_type(ValueType::Reference), value(value) {}
    explicit ValueRef(std::string* value) noexcept : m_type(ValueType::String), value(value) {}

    operator float&() const
    {
        utils::throw_runtime_error_if(m_type != ValueType::Float, "Conversion error");
        return *static_cast<float*>(value);
    }

    operator int&() const
    {
        utils::throw_runtime_error_if(m_type != ValueType::Int, "Conversion error");
        return *static_cast<int*>(value);
    }

    operator Object&() const
    {
        utils::throw_runtime_error_if(m_type != ValueType::Reference, "Conversion error");
        return *static_cast<Object*>(value);
    }

    operator std::string &() const
    {
        utils::throw_runtime_error_if(m_type != ValueType::String, "Conversion error");
        return *static_cast<std::string*>(value);
    }

    operator Float3&() const
    {
        utils::throw_runtime_error_if(m_type != ValueType::Float3, "Conversion error");
        return *static_cast<Float3*>(value);
    }

  private:
    using ValuePtr = void*;

    ValueType m_type = ValueType::Null;
    ValuePtr  value  = nullptr;
};

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
    const char* object_default_name{};
    const char* object_description{};

    std::vector<Field> fields;
};

inline bool IsReservedProperty(const std::string& property) noexcept
{
    return property == "object.name";
}

class Object {
  public:
    virtual ~Object() noexcept = default;

    auto GetID() const noexcept { return m_id; }

    auto GetName() const noexcept -> std::string;
    auto GetProperties() const -> DictionaryRef { return *m_dictionary; }
    bool HasProperties() const noexcept;

    void SetName(std::string name);
    void SetProperty(Key key, Value value);
    void RemoveProperty(Key key);

    virtual auto GetMetadata() const -> MetadataRef = 0;
    virtual auto ToJson() const -> json             = 0;

    virtual auto GetField(std::string_view field_name) -> ValueRef = 0;

  protected:
    friend struct ObjectAccess;

    using UniqueDictionary = std::unique_ptr<Dictionary>;

    Object(ID id) noexcept : m_id(id) {}

    ID               m_id;
    UniqueDictionary m_dictionary;
};

class MeshVertices final {
  public:
    template <Vertex T, typename A>
    MeshVertices(std::vector<T, A> vertices) : m_vertices(std::move(vertices))
    {
        using utils::narrow_cast;

        auto& vertices_ref = std::any_cast<std::vector<T>&>(m_vertices);

        vertices_ref.shrink_to_fit();

        m_vertex_attributes = to_string(vertex_flags<T>());
        m_vertex_size       = narrow_cast<int>(sizeof(vertices_ref[0]));
        m_count             = narrow_cast<int>(vertices_ref.size());
        m_size              = narrow_cast<int>(m_vertex_size * m_count);
        m_data              = static_cast<const void*>(vertices_ref.data());
    }

    MeshVertices(const MeshVertices&) = delete;
    MeshVertices& operator=(const MeshVertices&) = delete;

    MeshVertices(MeshVertices&&) = default;
    MeshVertices& operator=(MeshVertices&&) = default;

    auto VertexAttributesRef() noexcept -> std::string& { return m_vertex_attributes; }
    auto VertexSizeRef() noexcept -> int& { return m_vertex_size; }
    auto CountRef() noexcept -> int& { return m_count; }
    auto SizeRef() noexcept -> int& { return m_size; }

    auto GetVertexAttributes() const noexcept -> const std::string& { return m_vertex_attributes; }
    auto GetVertexSize() const noexcept { return m_vertex_size; }
    auto GetCount() const noexcept { return m_count; }
    auto GetSize() const noexcept { return m_size; }

    auto GetData() const noexcept { return m_data; }

  private:
    std::any    m_vertices{};
    std::string m_vertex_attributes{};
    int         m_vertex_size{};
    int         m_count{};
    int         m_size{};
    const void* m_data{};
};

class MeshIndices final {
  public:
    template <IndexType T>
    MeshIndices(std::vector<T> indices) : m_indices(indices)
    {
        using utils::narrow_cast;

        auto& indices_ref = std::any_cast<std::vector<T>&>(m_indices);

        indices_ref.shrink_to_fit();

        auto bits = 8 * sizeof(indices_ref[0]);

        m_type       = "int" + std::to_string(bits);
        m_index_size = narrow_cast<int>(sizeof(indices_ref[0]));
        m_count      = narrow_cast<int>(indices_ref.size());
        m_size       = narrow_cast<int>(m_index_size * m_count);
        m_data       = static_cast<const void*>(indices_ref.data());
    }

    MeshIndices(const MeshIndices&) = delete;
    MeshIndices& operator=(const MeshIndices&) = delete;

    MeshIndices(MeshIndices&&) = default;
    MeshIndices& operator=(MeshIndices&&) = default;

    auto IndexTypeRef() noexcept -> std::string& { return m_type; }
    auto IndexSizeRef() noexcept -> int& { return m_index_size; }
    auto CountRef() noexcept -> int& { return m_count; }
    auto SizeRef() noexcept -> int& { return m_size; }

    auto GetIndexType() const noexcept -> const std::string& { return m_type; }
    auto GetIndexSize() const noexcept { return m_index_size; }
    auto GetCount() const noexcept { return m_count; }
    auto GetSize() const noexcept { return m_size; }

    auto GetData() const noexcept { return m_data; }

  private:
    std::any    m_indices;
    std::string m_type;
    int         m_index_size = 0;
    int         m_count      = 0;
    int         m_size       = 0;
    const void* m_data       = nullptr;
};

class Mesh : public Object {
  public:
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    Mesh(Mesh&&) noexcept = default;
    Mesh& operator=(Mesh&&) noexcept = default;

    auto GetMetadata() const noexcept -> MetadataRef override { return metadata; }
    auto GetField(std::string_view field_name) -> ValueRef override;

    auto GetBoundingBox() const noexcept -> const AABB& { return m_aabb; }
    auto GetVertices() const noexcept -> const MeshVertices& { return m_vertices; }
    auto GetIndices() const noexcept -> const MeshIndices& { return m_indices; }

    json ToJson() const;

  private:
    friend struct ObjectAccess;

    Mesh(ID id, AABB aabb, MeshVertices mesh_vertices, MeshIndices mesh_indices) noexcept
        : Object(id), m_aabb(aabb), m_vertices(std::move(mesh_vertices)), m_indices(std::move(mesh_indices))
    {}

    static Metadata metadata;

    AABB         m_aabb{};
    MeshVertices m_vertices;
    MeshIndices  m_indices;
};

class Shader : public Object {
  public:
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    Shader(Shader&&) = default;
    Shader& operator=(Shader&&) = default;

    auto GetMetadata() const -> MetadataRef override { return metadata; }

    auto GetField(std::string_view) -> ValueRef override { return {}; }

    auto CreateMaterial() -> MaterialPtr;

    auto GetMaterials() const -> MaterialPtrArray;

    json ToJson() const override;

  protected:
    friend struct ObjectAccess;

    Shader(ID id) noexcept : Object(id){};

    inline static Metadata metadata = { "shader", "Shader", "Shader", {} };

    std::vector<UniqueMaterial> m_materials;
};

class Material final : public Shader {
  public:
    Material(const Material&) = delete;
    Material& operator=(const Material&) = delete;

    auto GetMetadata() const -> MetadataRef override { return metadata; }
    json ToJson() const override;

    auto GetInstanceNodes() const { return m_instance_nodes; }

    bool RemoveInstance(InstanceNodePtr node);

  private:
    friend struct ObjectAccess;

    Material(ID id) noexcept : Shader(id) {}

    void AddInstanceNodePtr(InstanceNodePtr mesh_instance_node);

    inline static Metadata metadata = { "material", "Material", "Material", {} };

    InstanceNodePtrArray m_instance_nodes;
};

class Node : public Object {
  public:
    Node(NodePtr parent, ID id) noexcept : Object(id), m_parent(parent) {}

    virtual bool IsRoot() const                      = 0;
    virtual bool IsInternal() const                  = 0;
    virtual bool IsLeaf() const                      = 0;
    virtual bool IsAncestor(NodePtr node) const      = 0;
    virtual bool HasChildren() const                 = 0;
    virtual auto GetChildren() const -> NodePtrArray = 0;

    virtual auto DetachNode() -> UniqueNode = 0;

  protected:
    friend struct ObjectAccess;

    virtual void ApplyTransform(const glm::mat4& matrix) noexcept = 0;

    NodePtr m_parent = nullptr;
};

class InternalNode : public Node {
  public:
    auto AddGroupNode() -> GroupNodePtr;
    auto AddTranslateNode(float x, float y, float z) -> TranslateNodePtr;
    auto AddRotateNode(float x, float y, float z, Radians angle) -> RotateNodePtr;
    auto AddScaleNode(float factor) -> ScaleNodePtr;
    auto AddInstanceNode(MeshPtr mesh, MaterialPtr material) -> InstanceNodePtr;

    void AttachNode(UniqueNode node);

    auto DetachNode() -> UniqueNode override;

    bool IsRoot() const override { return false; }
    bool IsInternal() const override { return true; }
    bool IsLeaf() const override { return false; }
    bool IsAncestor(NodePtr node) const override;
    bool HasChildren() const override;
    auto GetChildren() const -> NodePtrArray override;

  protected:
    friend struct ObjectAccess;

    InternalNode(NodePtr parent, ID id) noexcept : Node(parent, id) {}

    std::vector<UniqueNode> m_children;
};

class RootNode final : public InternalNode {
  public:
    RootNode(const RootNode&) = delete;
    RootNode& operator=(const RootNode&) = delete;

    auto GetMetadata() const -> MetadataRef override { return metadata; }

    auto GetField(std::string_view) -> ValueRef override { return {}; }

    bool IsRoot() const override { return true; }

    json ToJson() const override;

  private:
    friend struct ObjectAccess;

    static Metadata metadata;

    RootNode(NodePtr parent, ID id) noexcept : InternalNode(parent, id) {}

    virtual void ApplyTransform(const glm::mat4& matrix) noexcept override;
};

class GroupNode final : public InternalNode {
    GroupNode(const GroupNode&) = delete;
    GroupNode& operator=(const GroupNode&) = delete;

    auto GetMetadata() const -> MetadataRef override { return metadata; }

    auto GetField(std::string_view) -> ValueRef override { return {}; }

    json ToJson() const override;

  private:
    friend struct ObjectAccess;

    static Metadata metadata;

    GroupNode(NodePtr parent, ID id) noexcept : InternalNode(parent, id) {}

    virtual void ApplyTransform(const glm::mat4& matrix) noexcept override;
};

class TranslateNode final : public InternalNode {
    TranslateNode(const TranslateNode&) = delete;
    TranslateNode& operator=(const TranslateNode&) = delete;

    auto GetMetadata() const -> MetadataRef override { return metadata; }

    auto GetField(std::string_view field_name) -> ValueRef override;

    json ToJson() const override;

  private:
    friend struct ObjectAccess;

    static Metadata metadata;

    TranslateNode(NodePtr parent, ID id, float x, float y, float z) noexcept
        : InternalNode(parent, id), m_amount(x, y, z)
    {}

    virtual void ApplyTransform(const glm::mat4& matrix) noexcept override;

    Float3 m_amount;
};

class RotateNode final : public InternalNode {
  public:
    RotateNode(const RotateNode&) = delete;
    RotateNode& operator=(const RotateNode&) = delete;

    auto GetMetadata() const -> MetadataRef override { return metadata; }

    auto GetField(std::string_view field_name) -> ValueRef override;

    json ToJson() const override;

  private:
    friend struct ObjectAccess;

    static Metadata metadata;

    RotateNode(NodePtr parent, ID id, float x, float y, float z, Radians angle) noexcept
        : InternalNode(parent, id), m_axis(x, y, z), m_angle(angle)
    {}

    virtual void ApplyTransform(const glm::mat4& matrix) noexcept override;

    Float3  m_axis;
    Radians m_angle;
};

class ScaleNode final : public InternalNode {
  public:
    ScaleNode(const ScaleNode&) = delete;
    ScaleNode& operator=(const ScaleNode&) = delete;

    auto GetMetadata() const -> MetadataRef override { return metadata; }

    auto GetField(std::string_view field_name) -> ValueRef override;

    json ToJson() const override;

  private:
    friend struct ObjectAccess;

    static Metadata metadata;

    ScaleNode(NodePtr parent, ID id, float factor) noexcept : InternalNode(parent, id), m_factor(factor) {}

    virtual void ApplyTransform(const glm::mat4& matrix) noexcept override;

    float m_factor;
};

class InstanceNode final : public Node {
  public:
    InstanceNode(const InstanceNode&) = delete;
    InstanceNode& operator=(const InstanceNode&) = delete;

    ~InstanceNode() noexcept;

    auto GetMetadata() const -> MetadataRef override { return metadata; }
    auto GetField(std::string_view field_name) -> ValueRef override;
    bool IsRoot() const override { return false; }
    bool IsInternal() const override { return false; }
    bool IsLeaf() const override { return true; }
    bool IsAncestor(NodePtr node) const override;
    bool HasChildren() const override { return false; }
    auto GetChildren() const -> NodePtrArray override { return NodePtrArray{}; }
    json ToJson() const override;

    auto DetachNode() -> UniqueNode override;

    auto GetMeshPtr() const noexcept { return m_mesh; }
    auto GetMaterialPtr() const noexcept { return m_material; }
    auto GetTransformPtr() const noexcept -> const glm::mat4* { return &m_transform; }

  private:
    friend struct ObjectAccess;

    static Metadata metadata;

    InstanceNode(NodePtr parent, ID id, MeshPtr mesh, MaterialPtr material) noexcept;

    virtual void ApplyTransform(const glm::mat4& matrix) noexcept override;

    MeshPtr     m_mesh      = nullptr;
    MaterialPtr m_material  = nullptr;
    glm::mat4   m_transform = glm::mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
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

    auto CreateShader() -> ShaderPtr;

    auto CreateMesh(AABB aabb, MeshVertices mesh_vertices, MeshIndices mesh_indices) -> MeshPtr;

    auto ComputeDrawList() const -> DrawList;

    auto ComputeAxisAlignedBoundingBox() const -> AABB;

    json ToJson() const;

  private:
    UniqueShaderManager m_shader_manager;
    UniqueMeshManager   m_mesh_manager;
    UniqueNode          m_root_node;
};
