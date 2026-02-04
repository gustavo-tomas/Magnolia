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
                    std::unique_lock<std::mutex> lock(mutex);
                    map.clear();
                }

                b8 contains(const Key& key)
                {
                    std::unique_lock<std::mutex> lock(mutex);
                    return map.contains(key);
                }

                void erase(const Key& key)
                {
                    std::unique_lock<std::mutex> lock(mutex);
                    map.erase(key);
                }

                iterator find(const Key& key)
                {
                    std::unique_lock<std::mutex> lock(mutex);
                    return map.find(key);
                }

                iterator end()
                {
                    std::unique_lock<std::mutex> lock(mutex);
                    return map.end();
                }

                Value& operator[](const Key& key)
                {
                    std::unique_lock<std::mutex> lock(mutex);
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
                    std::unique_lock<std::mutex> lock(mutex);
                    queue.push(item);
                }

                T pop()
                {
                    std::unique_lock<std::mutex> lock(mutex);
                    T result = queue.front();
                    queue.pop();

                    return result;
                }

                void clear()
                {
                    std::unique_lock<std::mutex> lock(mutex);
                    while (!queue.empty())
                    {
                        queue.pop();
                    }
                }

                b8 empty()
                {
                    std::unique_lock<std::mutex> lock(mutex);
                    return queue.empty();
                }

                u64 size()
                {
                    std::unique_lock<std::mutex> lock(mutex);
                    return queue.size();
                }

            private:
                std::queue<T> queue;
                std::mutex mutex;
        };
    };  // namespace thread
};  // namespace mag
