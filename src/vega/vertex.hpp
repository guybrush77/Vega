#pragma once

#include "etna/core.hpp"
#include "platform.hpp"

BEGIN_DISABLE_WARNINGS

#include <glm/vec3.hpp>

END_DISABLE_WARNINGS

#include <type_traits>

DECLARE_VERTEX_ATTRIBUTE_TYPE(glm::vec3, etna::Format::R32G32B32Sfloat)

struct Vertex {};

template <typename T>
concept VertexType = std::is_base_of_v<Vertex, T>&& std::is_standard_layout_v<T>&& std::is_trivially_copyable_v<T>;

struct VertexPN : Vertex {
    constexpr VertexPN(const glm::vec3& position, const glm::vec3 normal) noexcept : position(position), normal(normal)
    {}
    glm::vec3 position;
    glm::vec3 normal;
};

template <typename T>
concept IndexType = std::is_same_v<T, uint16_t> || std::is_same_v<T, uint32_t>;
