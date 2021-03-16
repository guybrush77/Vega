#pragma once

#include <string>
#include <type_traits>

enum VertexFlags { Position3f = 1, Normal3f = 2 };

std::string to_string(VertexFlags value);

template <typename>
struct vertex_type_traits;

template <typename>
struct vertex_attribute_type_traits;

#define DECLARE_VERTEX_ATTRIBUTE_TYPE(attribute_type, equivalent_type) \
    template <> \
    struct vertex_attribute_type_traits<attribute_type> { \
        static constexpr auto value = equivalent_type; \
    };

#define formatof(vertex_type, field) vertex_attribute_type_traits<decltype(std::declval<vertex_type>().field)>::value

#define DECLARE_VERTEX_TYPE(vertex_type, vertex_attributes) \
    template <> \
    struct vertex_type_traits<vertex_type> { \
        static constexpr auto value = vertex_attributes; \
    };
