#include <array>

#include "magnolia/core/types.hpp"

namespace mag::stl
{
    // Fixed size pool with no memory allocations aside from the initial construction. Also has out-of-bounds checks to
    // prevent exceptions and segfaults. I think this implementation is pretty neat and could be reproduced for other
    // data structures.
    template <typename Resource, u32 _size>
    class pool
    {
        public:
            using Handle = u32;

            pool()
            {
                static_assert(_size > 0, "Pool size must be greater than zero");

                for (Handle i = 0; i < _size; i++)
                {
                    available_resources.at(i) = i;
                }
            }

            ~pool() = default;

            Handle acquire_resource()
            {
                MAG_ASSERT(first_available_resource < _size, "Pool size exceeded: '{}'. Please increase pool size.",
                           _size);
                if (first_available_resource >= _size)
                {
                    return 0;
                }

                const Handle handle = available_resources.at(first_available_resource);
                available_resources.at(first_available_resource) = Invalid_ID;
                first_available_resource++;

                return handle;
            }

            void release_resource(const Handle handle)
            {
                MAG_ASSERT(first_available_resource > 0, "Pool is already empty");
                MAG_ASSERT(handle < _size, "Out of bounds handle: '{0}' > (Max handle = '{1}')", handle, _size - 1);
                if (first_available_resource == 0 || handle >= _size)
                {
                    return;
                }

                available_resources.at(--first_available_resource) = handle;
            }

            b8 empty() const { return first_available_resource == 0; }

            u32 size() const { return _size; }

            u32 used_size() const { return first_available_resource; }

            constexpr Resource& operator[](const u32 i)
            {
                MAG_ASSERT(i < _size, "Out of bounds index: {0}", i);
                if (i >= _size)
                {
                    return resources[0];
                }

                return resources.at(i);
            }

        private:
            std::array<Resource, _size> resources;
            std::array<Handle, _size> available_resources;
            u64 first_available_resource = 0;
    };
};  // namespace mag::stl
