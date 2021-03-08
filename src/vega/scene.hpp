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
#include <map>
#include <memory>
#include <string_view>
#include <variant>
#include <vector>

class GroupNode;
class InnerNode;
class InstanceNode;
class Material;
class Mesh;
class Node;
class Object;
class RootNode;
class RotateNode;
class ScaleNode;
class Scene;
class Shader;
class TranslateNode;

using GroupNodePtr     = GroupNode*;
using InnerNodePtr     = InnerNode*;
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

using UniqueGroupNode     = std::unique_ptr<GroupNode>;
using UniqueInstanceNode  = std::unique_ptr<InstanceNode>;
using UniqueMaterial      = std::unique_ptr<Material>;
using UniqueMesh          = std::unique_ptr<Mesh>;
using UniqueNode          = std::unique_ptr<Node>;
using UniqueRotateNode    = std::unique_ptr<RotateNode>;
using UniqueScaleNode     = std::unique_ptr<ScaleNode>;
using UniqueShader        = std::unique_ptr<Shader>;
using UniqueTranslateNode = std::unique_ptr<TranslateNode>;

using Instances = std::vector<InstanceNodePtr>;
using Materials = std::vector<MaterialPtr>;
using Meshes    = std::vector<MeshPtr>;
using Nodes     = std::vector<NodePtr>;
using Shaders   = std::vector<ShaderPtr>;

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

using PropertyName  = std::string;
using PropertyValue = std::variant<std::monostate, int, float, Float3, std::string, ObjectPtr>;
using PropertyStore = std::map<PropertyName, PropertyValue>;

struct Property final {
    PropertyName  name;
    PropertyValue value;
};

using json = nlohmann::json;

class Object {
  public:
    virtual ~Object() noexcept = default;

    virtual auto GetProperty(std::string_view name) const -> PropertyValue                                  = 0;
    virtual auto GetProperty(std::string_view primary, std::string_view alternative) const -> PropertyValue = 0;
    virtual auto GetProperties() const -> std::vector<Property>                                             = 0;

    virtual bool SetProperty(std::string_view name, const PropertyValue& value) = 0;
    virtual bool RemoveProperty(std::string_view name)                          = 0;

    virtual auto ToJson() const -> json = 0;

    ID GetID() const noexcept { return m_id; }

  protected:
    friend struct ObjectAccess;

    Object(ID id) noexcept : m_id(id) {}

    ID            m_id;
    PropertyStore m_properties;
};

inline ID GetID(const Object* object) noexcept
{
    return object->GetID();
}

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

    auto GetProperty(std::string_view name) const -> PropertyValue override;
    auto GetProperty(std::string_view primary, std::string_view alternative) const -> PropertyValue override;
    auto GetProperties() const -> std::vector<Property> override;

    bool SetProperty(std::string_view name, const PropertyValue& value) override;
    bool RemoveProperty(std::string_view name) override;

    auto GetBoundingBox() const noexcept -> const AABB& { return m_aabb; }
    auto GetVertices() const noexcept -> const MeshVertices& { return m_vertices; }
    auto GetIndices() const noexcept -> const MeshIndices& { return m_indices; }

    json ToJson() const;

  private:
    friend struct ObjectAccess;

    static constexpr std::string_view                kClassName     = "mesh";
    static constexpr std::string_view                kDefaultName   = "Mesh";
    static constexpr std::array<std::string_view, 4> kFieldNames    = { "Min", "Max", "Vertices", "Faces" };
    static constexpr std::array<bool, 4>             kFieldWritable = { false, false, false, false };

    Mesh(ID id, AABB aabb, MeshVertices mesh_vertices, MeshIndices mesh_indices) noexcept
        : Object(id), m_aabb(aabb), m_vertices(std::move(mesh_vertices)), m_indices(std::move(mesh_indices))
    {}

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

    auto GetProperty(std::string_view name) const -> PropertyValue override;
    auto GetProperty(std::string_view primary, std::string_view alternative) const -> PropertyValue override;
    auto GetProperties() const -> std::vector<Property> override;

    bool SetProperty(std::string_view name, const PropertyValue& value) override;
    bool RemoveProperty(std::string_view name) override;

    auto CreateMaterial() -> MaterialPtr;

    auto GetMaterials() const -> Materials;

    json ToJson() const override;

  protected:
    friend struct ObjectAccess;

    static constexpr std::string_view                kClassName     = "shader";
    static constexpr std::string_view                kDefaultName   = "Shader";
    static constexpr std::array<std::string_view, 0> kFieldNames    = {};
    static constexpr std::array<bool, 0>             kFieldWritable = {};

    Shader(ID id) noexcept : Object(id){};

    std::vector<UniqueMaterial> m_materials;
};

class Material final : public Shader {
  public:
    Material(const Material&) = delete;
    Material& operator=(const Material&) = delete;

    json ToJson() const override;

    auto GetInstanceNodes() const { return m_instances; }

    bool RemoveInstance(InstanceNodePtr node);

  private:
    friend struct ObjectAccess;

    static constexpr std::string_view kClassName   = "material";
    static constexpr std::string_view kDefaultName = "Material";

    Material(ID id) noexcept : Shader(id) {}

    void AddInstanceNodePtr(InstanceNodePtr mesh_instance_node);

    Instances m_instances;
};

class Node : public Object {
  public:
    virtual auto AttachNode(UniqueNode node) -> NodePtr = 0;
    virtual auto DetachNode() -> UniqueNode             = 0;

    virtual bool IsRoot() const                 = 0;
    virtual bool IsInner() const                = 0;
    virtual bool IsLeaf() const                 = 0;
    virtual bool IsAncestor(NodePtr node) const = 0;
    virtual bool HasChildren() const            = 0;
    virtual auto GetChildren() const -> Nodes   = 0;

  protected:
    friend struct ObjectAccess;

    Node(NodePtr parent, ID id) noexcept : Object(id), m_parent(parent) {}

    virtual void ApplyTransform(const glm::mat4& matrix) noexcept = 0;

    NodePtr m_parent = nullptr;
};

class InnerNode : public Node {
  public:
    auto AttachNode(UniqueNode node) -> NodePtr override;
    auto DetachNode() -> UniqueNode override;

    bool IsRoot() const override { return false; }
    bool IsInner() const override { return true; }
    bool IsLeaf() const override { return false; }
    bool IsAncestor(NodePtr node) const override;
    bool HasChildren() const override;
    auto GetChildren() const -> Nodes override;

  protected:
    friend struct ObjectAccess;

    InnerNode(NodePtr parent, ID id) noexcept : Node(parent, id) {}

    std::vector<UniqueNode> m_children;
};

class RootNode final : public InnerNode {
  public:
    RootNode(const RootNode&) = delete;
    RootNode& operator=(const RootNode&) = delete;

    auto GetProperty(std::string_view name) const -> PropertyValue override;
    auto GetProperty(std::string_view primary, std::string_view alternative) const -> PropertyValue override;
    auto GetProperties() const -> std::vector<Property> override;

    bool SetProperty(std::string_view name, const PropertyValue& value) override;
    bool RemoveProperty(std::string_view name) override;

    auto DetachNode() -> UniqueNode override;

    bool IsRoot() const override { return true; }

    json ToJson() const override;

  private:
    friend struct ObjectAccess;

    static constexpr std::string_view                kClassName     = "root.node";
    static constexpr std::string_view                kDefaultName   = "Root";
    static constexpr std::array<std::string_view, 0> kFieldNames    = {};
    static constexpr std::array<bool, 0>             kFieldWritable = {};

    RootNode(NodePtr parent, ID id) noexcept : InnerNode(parent, id) {}

    virtual void ApplyTransform(const glm::mat4& matrix) noexcept override;
};

class GroupNode final : public InnerNode {
    GroupNode(const GroupNode&) = delete;
    GroupNode& operator=(const GroupNode&) = delete;

    auto GetProperty(std::string_view name) const -> PropertyValue override;
    auto GetProperty(std::string_view primary, std::string_view alternative) const -> PropertyValue override;
    auto GetProperties() const -> std::vector<Property> override;

    bool SetProperty(std::string_view name, const PropertyValue& value) override;
    bool RemoveProperty(std::string_view name) override;

    json ToJson() const override;

  private:
    friend struct ObjectAccess;

    static constexpr std::string_view                kClassName     = "group.node";
    static constexpr std::string_view                kDefaultName   = "Group";
    static constexpr std::array<std::string_view, 0> kFieldNames    = {};
    static constexpr std::array<bool, 0>             kFieldWritable = {};

    GroupNode(NodePtr parent, ID id) noexcept : InnerNode(parent, id) {}

    virtual void ApplyTransform(const glm::mat4& matrix) noexcept override;
};

class TranslateNode final : public InnerNode {
    TranslateNode(const TranslateNode&) = delete;
    TranslateNode& operator=(const TranslateNode&) = delete;

    auto GetProperty(std::string_view name) const -> PropertyValue override;
    auto GetProperty(std::string_view primary, std::string_view alternative) const -> PropertyValue override;
    auto GetProperties() const -> std::vector<Property> override;

    bool SetProperty(std::string_view name, const PropertyValue& value) override;
    bool RemoveProperty(std::string_view name) override;

    json ToJson() const override;

  private:
    friend struct ObjectAccess;

    static constexpr std::string_view                kClassName     = "translate.node";
    static constexpr std::string_view                kDefaultName   = "Translate";
    static constexpr std::array<std::string_view, 1> kFieldNames    = { "Distance" };
    static constexpr std::array<bool, 1>             kFieldWritable = { true };

    TranslateNode(NodePtr parent, ID id, Float3 distance) noexcept : InnerNode(parent, id), m_distance(distance) {}

    virtual void ApplyTransform(const glm::mat4& matrix) noexcept override;

    Float3 m_distance;
};

class RotateNode final : public InnerNode {
  public:
    RotateNode(const RotateNode&) = delete;
    RotateNode& operator=(const RotateNode&) = delete;

    auto GetProperty(std::string_view name) const -> PropertyValue override;
    auto GetProperty(std::string_view primary, std::string_view alternative) const -> PropertyValue override;
    auto GetProperties() const -> std::vector<Property> override;

    bool SetProperty(std::string_view name, const PropertyValue& value) override;
    bool RemoveProperty(std::string_view name) override;

    json ToJson() const override;

  private:
    friend struct ObjectAccess;

    static constexpr std::string_view                kClassName     = "rotate.node";
    static constexpr std::string_view                kDefaultName   = "Rotate";
    static constexpr std::array<std::string_view, 2> kFieldNames    = { "Axis", "Angle" };
    static constexpr std::array<bool, 2>             kFieldWritable = { true, true };

    RotateNode(NodePtr parent, ID id, Float3 axis, Radians angle) noexcept
        : InnerNode(parent, id), m_axis(axis), m_angle(angle)
    {}

    virtual void ApplyTransform(const glm::mat4& matrix) noexcept override;

    Float3  m_axis;
    Radians m_angle;
};

class ScaleNode final : public InnerNode {
  public:
    ScaleNode(const ScaleNode&) = delete;
    ScaleNode& operator=(const ScaleNode&) = delete;

    auto GetProperty(std::string_view name) const -> PropertyValue override;
    auto GetProperty(std::string_view primary, std::string_view alternative) const -> PropertyValue override;
    auto GetProperties() const -> std::vector<Property> override;

    bool SetProperty(std::string_view name, const PropertyValue& value) override;
    bool RemoveProperty(std::string_view name) override;

    json ToJson() const override;

  private:
    friend struct ObjectAccess;

    static constexpr std::string_view                kClassName     = "scale.node";
    static constexpr std::string_view                kDefaultName   = "Scale";
    static constexpr std::array<std::string_view, 1> kFieldNames    = { "Factor" };
    static constexpr std::array<bool, 1>             kFieldWritable = { true };

    ScaleNode(NodePtr parent, ID id, float factor) noexcept : InnerNode(parent, id), m_factor(factor) {}

    virtual void ApplyTransform(const glm::mat4& matrix) noexcept override;

    float m_factor;
};

class InstanceNode final : public Node {
  public:
    InstanceNode(const InstanceNode&) = delete;
    InstanceNode& operator=(const InstanceNode&) = delete;

    ~InstanceNode() noexcept;

    auto GetProperty(std::string_view name) const -> PropertyValue override;
    auto GetProperty(std::string_view primary, std::string_view alternative) const -> PropertyValue override;
    auto GetProperties() const -> std::vector<Property> override;

    bool SetProperty(std::string_view name, const PropertyValue& value) override;
    bool RemoveProperty(std::string_view name) override;

    auto AttachNode(UniqueNode node) -> NodePtr override;
    auto DetachNode() -> UniqueNode override;

    bool IsRoot() const override { return false; }
    bool IsInner() const override { return false; }
    bool IsLeaf() const override { return true; }
    bool IsAncestor(NodePtr node) const override;
    bool HasChildren() const override { return false; }
    auto GetChildren() const -> Nodes override { return Nodes{}; }

    json ToJson() const override;

    auto GetMeshPtr() const noexcept { return m_mesh; }
    auto GetMaterialPtr() const noexcept { return m_material; }
    auto GetTransformPtr() const noexcept -> const glm::mat4* { return &m_transform; }

  private:
    friend struct ObjectAccess;

    static constexpr std::string_view                kClassName     = "instance.node";
    static constexpr std::string_view                kDefaultName   = "Mesh Instance";
    static constexpr std::array<std::string_view, 2> kFieldNames    = { "Mesh", "Material" };
    static constexpr std::array<bool, 2>             kFieldWritable = { false, false };

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

    auto GetRootNode() noexcept -> RootNodePtr;

    auto CreateGroupNode() -> UniqueGroupNode;
    auto CreateTranslateNode(Float3 distance) -> UniqueTranslateNode;
    auto CreateRotateNode(Float3 axis, Radians angle) -> UniqueRotateNode;
    auto CreateScaleNode(float factor) -> UniqueScaleNode;
    auto CreateInstanceNode(MeshPtr mesh, MaterialPtr material) -> UniqueInstanceNode;

    auto CreateShader() -> ShaderPtr;

    auto CreateMesh(AABB aabb, MeshVertices vertices, MeshIndices indices) -> MeshPtr;

    auto ComputeDrawList() const -> DrawList;

    auto ComputeAxisAlignedBoundingBox() const -> AABB;

    json ToJson() const;

  private:
    std::vector<UniqueShader> m_shaders;
    std::vector<UniqueMesh>   m_meshes;
    UniqueNode                m_root;
};
