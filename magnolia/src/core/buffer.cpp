#include "core/buffer.hpp"

namespace mag
{
    Buffer::Buffer() = default;
    Buffer::Buffer(const u64 size) : data(size) {}

    u64 Buffer::get_size() const { return data.size(); }
};  // namespace mag
