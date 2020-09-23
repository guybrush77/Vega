#include <type_traits>

void throw_runtime_error(const char* description);

template <class T, class U>
struct have_same_sign : std::integral_constant<bool, std::is_signed<T>::value == std::is_signed<U>::value>
{ };

template <class DstT, class SrcT>
constexpr DstT narrow_cast(SrcT src)
{
    DstT dst = static_cast<DstT>(src);

    if (static_cast<SrcT>(dst) != src)
    {
        throw_runtime_error("narrow_cast failed");
    }

    if (!have_same_sign<DstT, SrcT>::value && ((dst < DstT{}) != (src < SrcT{})))
    {
        throw_runtime_error("narrow_cast failed");
    }

    return dst;
}
