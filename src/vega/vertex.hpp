#pragma once

#include "etna/core.hpp"
#include "platform.hpp"

BEGIN_DISABLE_WARNINGS

#include <glm/vec3.hpp>

END_DISABLE_WARNINGS

DECLARE_VERTEX_ATTRIBUTE_TYPE(glm::vec3, etna::Format::R32G32B32Sfloat)

struct Vertex {};

template <typename T>
concept VertexType = std::is_base_of_v<Vertex, T>&& std::is_standard_layout_v<T>&& std::is_trivially_copyable_v<T>;

struct VertexPN : Vertex {
    glm::vec3 position;
    glm::vec3 normal;
};
