#pragma once

#include "platform.hpp"
#include "utils/cast.hpp"
#include "utils/math.hpp"
#include "utils/misc.hpp"
#include "vertex.hpp"

BEGIN_DISABLE_WARNINGS

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtx/transform.hpp>
#include <glm/matrix.hpp>

END_DISABLE_WARNINGS

#include <nlohmann/json.hpp>

#include <map>
#include <memory>
#include <string_view>
#include <variant>
#include <vector>

class Buffer;
class GroupNode;
class IndexBuffer;
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
class VertexBuffer;

using BufferPtr        = Buffer*;
using GroupNodePtr     = GroupNode*;
using IndexBufferPtr   = IndexBuffer*;
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
using VertexBufferPtr  = VertexBuffer*;

using UniqueBuffer        = std::unique_ptr<Buffer>;
using UniqueGroupNode     = std::unique_ptr<GroupNode>;
using UniqueInstanceNode  = std::unique_ptr<InstanceNode>;
using UniqueMaterial      = std::unique_ptr<Material>;
using UniqueMesh          = std::unique_ptr<Mesh>;
using UniqueNode          = std::unique_ptr<Node>;
using UniqueObject        = std::unique_ptr<Object>;
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

using PropertyName = std::string;
using PropertyValue =
    std::variant<std::monostate, int32_t, int64_t, uint32_t, uint64_t, float, Float3, std::string, ObjectPtr>;
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

class Buffer : public Object {
  public:
    Buffer(ID id) noexcept : Object(id) {}

    auto Data() const noexcept { return m_data.get(); }
    auto Size() const noexcept { return m_size; }

  protected:
    Buffer(ID id, void* src, size_t size, std::align_val_t alignment);

    struct Deleter final {
        void             operator()(void* data) { ::operator delete(data, alignment); };
        std::align_val_t alignment{};
    };

    std::unique_ptr<void, Deleter> m_data{};
    size_t                         m_size{};
    Deleter                        m_deleter;
};

class VertexBuffer final : public Buffer {
  public:
    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer& operator=(const VertexBuffer&) = delete;

    VertexBuffer(VertexBuffer&&) noexcept = default;
    VertexBuffer& operator=(VertexBuffer&&) noexcept = default;

    auto GetProperty(std::string_view name) const -> PropertyValue override;
    auto GetProperty(std::string_view primary, std::string_view alternative) const -> PropertyValue override;
    auto GetProperties() const -> std::vector<Property> override;

    bool SetProperty(std::string_view name, const PropertyValue& value) override;
    bool RemoveProperty(std::string_view name) override;

    json ToJson() const override;

  private:
    friend struct ObjectAccess;

    static constexpr std::string_view                kClassName     = "vertex.buffer";
    static constexpr std::string_view                kDefaultName   = "Vertex Buffer";
    static constexpr std::array<std::string_view, 1> kFieldNames    = { "Size" };
    static constexpr std::array<bool, 1>             kFieldWritable = { false };

    VertexBuffer(ID id, void* src, size_t size, std::align_val_t alignment) : Buffer(id, src, size, alignment) {}
};

class IndexBuffer final : public Buffer {
  public:
    IndexBuffer(const IndexBuffer&) = delete;
    IndexBuffer& operator=(const IndexBuffer&) = delete;

    IndexBuffer(IndexBuffer&&) noexcept = default;
    IndexBuffer& operator=(IndexBuffer&&) noexcept = default;

    auto GetProperty(std::string_view name) const -> PropertyValue override;
    auto GetProperty(std::string_view primary, std::string_view alternative) const -> PropertyValue override;
    auto GetProperties() const -> std::vector<Property> override;

    bool SetProperty(std::string_view name, const PropertyValue& value) override;
    bool RemoveProperty(std::string_view name) override;

    json ToJson() const override;

  private:
    friend struct ObjectAccess;

    static constexpr std::string_view                kClassName     = "index.buffer";
    static constexpr std::string_view                kDefaultName   = "Index Buffer";
    static constexpr std::array<std::string_view, 1> kFieldNames    = { "Size" };
    static constexpr std::array<bool, 1>             kFieldWritable = { false };

    IndexBuffer(ID id, void* src, size_t size, std::align_val_t alignment) : Buffer(id, src, size, alignment) {}
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

    auto GetBoundingBox() const noexcept { return m_aabb; }
    auto GetVertexBuffer() const noexcept { return m_vertex_buffer; }
    auto GetIndexBuffer() const noexcept { return m_index_buffer; }
    auto GetFirstIndex() const noexcept { return m_first_index; }
    auto GetIndexCount() const noexcept { return m_index_count; }

    json ToJson() const;

  private:
    friend struct ObjectAccess;

    static constexpr std::string_view                kClassName     = "mesh";
    static constexpr std::string_view                kDefaultName   = "Mesh";
    static constexpr std::array<std::string_view, 3> kFieldNames    = { "Triangles", "Min", "Max" };
    static constexpr std::array<bool, 3>             kFieldWritable = { false, false, false };

    Mesh(
        ID              id,
        AABB            aabb,
        VertexBufferPtr vertex_buffer,
        IndexBufferPtr  index_buffer,
        size_t          first_index,
        size_t          index_count) noexcept
        : Object(id), m_aabb(aabb), m_vertex_buffer(vertex_buffer), m_index_buffer(index_buffer),
          m_first_index(first_index), m_index_count(index_count)
    {}

    AABB            m_aabb{};
    VertexBufferPtr m_vertex_buffer;
    IndexBufferPtr  m_index_buffer;
    size_t          m_first_index;
    size_t          m_index_count;
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

    auto GetMaterials() const -> Materials;

    json ToJson() const override;

  protected:
    friend struct ObjectAccess;

    static constexpr std::string_view                kClassName     = "shader";
    static constexpr std::string_view                kDefaultName   = "Shader";
    static constexpr std::array<std::string_view, 0> kFieldNames    = {};
    static constexpr std::array<bool, 0>             kFieldWritable = {};

    Shader(ID id) noexcept : Object(id){};

    void AddMaterialPtr(MaterialPtr material);

    Materials m_materials;
};

class Material final : public Object {
  public:
    Material(const Material&) = delete;
    Material& operator=(const Material&) = delete;

    auto GetProperty(std::string_view name) const -> PropertyValue override;
    auto GetProperty(std::string_view primary, std::string_view alternative) const -> PropertyValue override;
    auto GetProperties() const -> std::vector<Property> override;

    bool SetProperty(std::string_view name, const PropertyValue& value) override;
    bool RemoveProperty(std::string_view name) override;

    json ToJson() const override;

    auto GetInstanceNodes() const { return m_instances; }

    bool RemoveInstance(InstanceNodePtr node);

  private:
    friend struct ObjectAccess;

    static constexpr std::string_view                kClassName     = "material";
    static constexpr std::string_view                kDefaultName   = "Material";
    static constexpr std::array<std::string_view, 0> kFieldNames    = {};
    static constexpr std::array<bool, 0>             kFieldWritable = {};

    Material(ID id) noexcept : Object(id) {}

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

    Node(ID id, NodePtr parent) noexcept : Object(id), m_parent(parent) {}

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

    InnerNode(ID id, NodePtr parent) noexcept : Node(id, parent) {}

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

    RootNode(ID id, NodePtr parent) noexcept : InnerNode(id, parent) {}

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

    GroupNode(ID id, NodePtr parent) noexcept : InnerNode(id, parent) {}

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

    TranslateNode(ID id, NodePtr parent, Float3 distance) noexcept : InnerNode(id, parent), m_distance(distance) {}

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

    RotateNode(ID id, NodePtr parent, Float3 axis, Radians angle) noexcept
        : InnerNode(id, parent), m_axis(axis), m_angle(angle)
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

    ScaleNode(ID id, NodePtr parent, float factor) noexcept : InnerNode(id, parent), m_factor(factor) {}

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
    auto GetTransform() const noexcept { return m_transform; }

  private:
    friend struct ObjectAccess;

    static constexpr std::string_view                kClassName     = "instance.node";
    static constexpr std::string_view                kDefaultName   = "Mesh Instance";
    static constexpr std::array<std::string_view, 2> kFieldNames    = { "Mesh", "Material" };
    static constexpr std::array<bool, 2>             kFieldWritable = { false, false };

    InstanceNode(ID id, NodePtr parent, MeshPtr mesh, MaterialPtr material) noexcept;

    virtual void ApplyTransform(const glm::mat4& matrix) noexcept override;

    MeshPtr     m_mesh      = nullptr;
    MaterialPtr m_material  = nullptr;
    glm::mat4   m_transform = glm::mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
};

struct DrawRecord final {
    size_t    index{};
    MeshPtr   mesh{};
    glm::mat4 transform{};
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

    auto CreateVertexBuffer(void* data, size_t size, std::align_val_t alignment) -> VertexBufferPtr;
    auto CreateIndexBuffer(void* data, size_t size, std::align_val_t alignment) -> IndexBufferPtr;

    auto CreateShader() -> ShaderPtr;
    auto CreateMaterial(ShaderPtr shader) -> MaterialPtr;

    auto CreateMesh(
        AABB            aabb,
        VertexBufferPtr vertex_buffer,
        IndexBufferPtr  index_buffer,
        size_t          first_index,
        size_t          index_count) -> MeshPtr;

    auto ComputeDrawList() const -> DrawList;

    auto ComputeAxisAlignedBoundingBox() const -> AABB;

    json ToJson() const;

  private:
    std::vector<ShaderPtr>       m_shaders;
    std::vector<MaterialPtr>     m_materials;
    std::vector<MeshPtr>         m_meshes;
    std::vector<VertexBufferPtr> m_vertex_buffers;
    std::vector<IndexBufferPtr>  m_index_buffers;
    std::map<ID, UniqueObject>   m_objects;
    UniqueNode                   m_root;
};
