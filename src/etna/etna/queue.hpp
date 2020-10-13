#pragma once

#include "types.hpp"

namespace etna {

class Queue {
  public:
    Queue() noexcept {}
    Queue(std::nullptr_t) noexcept {}
    Queue(VkQueue queue) noexcept : m_queue(queue) {}

    operator VkQueue() const noexcept { return m_queue; }

    explicit operator bool() const noexcept { return m_queue != nullptr; }

    bool operator==(const Queue& rhs) const noexcept { return m_queue == rhs.m_queue; }
    bool operator!=(const Queue& rhs) const noexcept { return m_queue != rhs.m_queue; }

    void Submit(CommandBuffer command_buffer);

  private:
    VkQueue m_queue{};
};

} // namespace etna
