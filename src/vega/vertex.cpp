#include "vertex.hpp"

#include "utils/misc.hpp"

template <typename T>
static std::string build_string(T value)
{
    std::string out;

    auto mask  = static_cast<std::underlying_type_t<T>>(value);
    bool first = true;

    while (mask) {
        auto rightmost = mask & -mask;
        if (false == first) {
            out.append(", ");
        }
        out.append(to_string_helper(static_cast<T>(rightmost)));
        mask  = mask ^ rightmost;
        first = false;
    }

    return out;
}

static const char* to_string_helper(VertexFlags value)
{
    switch (value) {
    case Position3f: return "Position3f";
    case Normal3f: return "Normal3f";
    default: utils::throw_runtime_error("Bad Enum");
    }
    return nullptr;
}

std::string to_string(VertexFlags value)
{
    return build_string(value);
}
