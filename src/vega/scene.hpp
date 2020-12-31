#pragma once

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

class Material;
class MaterialInstance;
class Mesh;
class MaterialManager;
class MeshManager;
class MeshNode;
class Node;
class Object;
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
using ObjectPtr           = Object*;
using RootNodePtr         = RootNode*;
using RotateNodePtr       = RotateNode*;
using ScaleNodePtr        = ScaleNode*;
using ScenePtr            = Scene*;
using TranslateNodePtr    = TranslateNode*;

using UniqueMaterial         = std::unique_ptr<Material>;
using UniqueMaterialInstance = std::unique_ptr<MaterialInstance>;
using UniqueMesh             = std::unique_ptr<Mesh>;
using UniqueMaterialManager  = std::unique_ptr<MaterialManager>;
using UniqueMeshManager      = std::unique_ptr<MeshManager>;
using UniqueNode             = std::unique_ptr<Node>;

using MaterialPtrArray         = std::vector<MaterialPtr>;
using MaterialInstancePtrArray = std::vector<MaterialInstancePtr>;
using MeshNodePtrArray         = std::vector<MeshNodePtr>;
using MeshPtrArray             = std::vector<MeshPtr>;
using NodePtrArray             = std::vector<NodePtr>;
using ObjectPtrArray           = std::vector<ObjectPtr>;

using MaterialRefArray         = std::vector<std::reference_wrapper<Material>>;
using MaterialInstanceRefArray = std::vector<std::reference_wrapper<MaterialInstance>>;
using MeshRefArray             = std::vector<std::reference_wrapper<Mesh>>;
using NodeRefArray             = std::vector<std::reference_wrapper<Node>>;
using ObjectRefArray           = std::vector<std::reference_wrapper<Object>>;

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

enum class ValueType { Null, Float, Int, Reference, String, Vec3 };

std::string to_string(ValueType value_type);

class ValueRef final {
  public:
    ValueRef() noexcept = default;

    explicit ValueRef(float* value) noexcept : m_type(ValueType::Float), value(value) {}
    explicit ValueRef(int* value) noexcept : m_type(ValueType::Int), value(value) {}
    explicit ValueRef(Object* value) noexcept : m_type(ValueType::Reference), value(value) {}
    explicit ValueRef(std::string* value) noexcept : m_type(ValueType::String), value(value) {}
    explicit ValueRef(glm::vec3* value) noexcept : m_type(ValueType::Vec3), value(value) {}

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

    operator glm::vec3 &() const
    {
        utils::throw_runtime_error_if(m_type != ValueType::Vec3, "Conversion error");
        return *static_cast<glm::vec3*>(value);
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
    const char* object_label{};
    const char* object_description{};

    std::vector<Field> fields;
};

class Object {
  public:
    virtual ~Object() noexcept = default;

    auto GetID() const noexcept { return m_id; }

    auto Properties() const -> DictionaryRef { return *m_dictionary; }
    bool HasProperties() const noexcept;
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

class Material : public Object {
  public:
    Material(const Material&) = delete;
    Material& operator=(const Material&) = delete;

    Material(Material&&) = default;
    Material& operator=(Material&&) = default;

    auto GetMetadata() const -> MetadataRef override { return metadata; }

    auto GetField(std::string_view) -> ValueRef override { return {}; }

    auto CreateMaterialInstance() -> MaterialInstancePtr;

    auto GetMaterialInstances() const -> MaterialInstancePtrArray;

    json ToJson() const override;

  protected:
    friend struct ObjectAccess;

    Material(ID id) noexcept : Object(id){};

    inline static Metadata metadata = { "material", "Material", nullptr, {} };

    std::vector<UniqueMaterialInstance> m_instances;
};

class MaterialInstance final : public Material {
  public:
    MaterialInstance(const MaterialInstance&) = delete;
    MaterialInstance& operator=(const MaterialInstance&) = delete;

    auto GetMetadata() const -> MetadataRef override { return metadata; }
    json ToJson() const override;

    auto GetMeshNodes() const { return m_mesh_nodes; }

  private:
    friend struct ObjectAccess;

    MaterialInstance(ID id) noexcept : Material(id) {}

    void AddMeshNodePtr(MeshNodePtr mesh_node);

    inline static Metadata metadata = { "material.instance", "Material Instance", nullptr, {} };

    MeshNodePtrArray m_mesh_nodes;
};

class Node : public Object {
  public:
    Node(ID id) noexcept : Object(id) {}

    virtual auto GetChildren() const -> NodePtrArray = 0;

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

    auto GetChildren() const -> NodePtrArray override;

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

    auto GetField(std::string_view) -> ValueRef override;

    json ToJson() const override;

  private:
    friend struct ObjectAccess;

    static Metadata metadata;

    RootNode(ID id) noexcept : InternalNode(id) {}

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

    TranslateNode(ID id, glm::vec3 amount) noexcept : InternalNode(id), m_amount(amount) {}

    virtual void ApplyTransform(const glm::mat4& matrix) noexcept override;

    glm::vec3 m_amount;
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

    auto GetField(std::string_view field_name) -> ValueRef override;

    json ToJson() const override;

  private:
    friend struct ObjectAccess;

    static Metadata metadata;

    ScaleNode(ID id, float factor) noexcept : InternalNode(id), m_factor(factor) {}

    virtual void ApplyTransform(const glm::mat4& matrix) noexcept override;

    float m_factor;
};

class MeshNode final : public Node {
  public:
    MeshNode(const MeshNode&) = delete;
    MeshNode& operator=(const MeshNode&) = delete;

    auto GetMetadata() const -> MetadataRef override { return metadata; }
    auto GetField(std::string_view field_name) -> ValueRef override;
    auto GetChildren() const -> NodePtrArray override { return NodePtrArray{}; }
    json ToJson() const override;

    auto GetMeshPtr() const noexcept { return m_mesh; }
    auto GetMaterialInstancePtr() const noexcept { return m_material_instance; }
    auto GetTransformPtr() const noexcept -> const glm::mat4* { return &m_transform; }

  private:
    friend struct ObjectAccess;

    static Metadata metadata;

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
