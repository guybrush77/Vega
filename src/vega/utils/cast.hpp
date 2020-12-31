#pragma once

#include <type_traits>

namespace detail {

template <typename T, typename U>
struct have_same_sign : std::integral_constant<bool, std::is_signed<T>::value == std::is_signed<U>::value> {};

void throw_runtime_error_impl(const char* message);

} // namespace detail

namespace utils {

template <typename DstT, typename SrcT>
constexpr DstT narrow_cast(SrcT src)
{
    DstT dst = static_cast<DstT>(src);

    if constexpr (!std::is_same_v<SrcT, DstT>) {
        if (static_cast<SrcT>(dst) != src) {
            detail::throw_runtime_error_impl("narrow_cast error");
        }
    }
    if constexpr (!detail::have_same_sign<DstT, SrcT>::value) {
        if ((dst < DstT{}) != (src < SrcT{})) {
            detail::throw_runtime_error_impl("narrow_cast error");
        }
    }

    return dst;
}

} // namespace utils
