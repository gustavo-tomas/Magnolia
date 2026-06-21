#pragma once

#include <mutex>
#include <queue>
#include <unordered_map>

#include "magnolia/core/types.hpp"

// Thread safe containers with mutual exclusion.

namespace mag
{
    namespace thread
    {
        template <typename Key, typename Value>
        class MAG_API Map
        {
            public:
                using iterator = typename std::unordered_map<Key, Value>::iterator;

                Map() = default;

                ~Map()
                {
                    const std::unique_lock<std::mutex> lock(mutex);
                    map.clear();
                }

                b8 contains(const Key& key)
                {
                    const std::unique_lock<std::mutex> lock(mutex);
                    return map.contains(key);
                }

                void erase(const Key& key)
                {
                    const std::unique_lock<std::mutex> lock(mutex);
                    map.erase(key);
                }

                iterator find(const Key& key)
                {
                    const std::unique_lock<std::mutex> lock(mutex);
                    return map.find(key);
                }

                iterator begin()
                {
                    const std::unique_lock<std::mutex> lock(mutex);
                    return map.begin();
                }

                iterator end()
                {
                    const std::unique_lock<std::mutex> lock(mutex);
                    return map.end();
                }

                Value& operator[](const Key& key)
                {
                    const std::unique_lock<std::mutex> lock(mutex);
                    return map[key];
                }

            private:
                std::unordered_map<Key, Value> map;
                std::mutex mutex;
        };

        template <typename T>
        class MAG_API Queue
        {
            public:
                Queue() = default;

                ~Queue() { clear(); }

                void push(const T& item)
                {
                    const std::unique_lock<std::mutex> lock(mutex);
                    queue.push(item);
                }

                T pop()
                {
                    const std::unique_lock<std::mutex> lock(mutex);
                    T result = queue.front();
                    queue.pop();

                    return result;
                }

                void clear()
                {
                    const std::unique_lock<std::mutex> lock(mutex);
                    while (!queue.empty())
                    {
                        queue.pop();
                    }
                }

                b8 empty()
                {
                    const std::unique_lock<std::mutex> lock(mutex);
                    return queue.empty();
                }

                u64 size()
                {
                    const std::unique_lock<std::mutex> lock(mutex);
                    return queue.size();
                }

            private:
                std::queue<T> queue;
                std::mutex mutex;
        };
    };  // namespace thread
};  // namespace mag
