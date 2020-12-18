#pragma once

template <typename OutPtr>
OutPtr pointer_cast(void* ptr) noexcept
{
    return static_cast<OutPtr>(ptr);
}
