#pragma once

#include "magnolia/core/logger.hpp"
#include "magnolia/math/types.hpp"
#include "magnolia/platform/file_system.hpp"
#include "nlohmann/json.hpp"

namespace mag::fs
{
    using json = nlohmann::ordered_json;

    MAG_API b8 read_json_data(const fs::path& file_path, fs::json& data);
    MAG_API b8 write_json_data(const fs::path& file_path, const fs::json& data);
};  // namespace mag::fs

namespace nlohmann
{
    template <typename T>
    inline void to_json(mag::fs::json& data, const mag::math::vector2<T>& v)
    {
        data = mag::fs::json::array();

        for (u32 i = 0; i < 2; i++)
        {
            data.push_back(v[i]);
        }
    }

    template <typename T>
    inline void to_json(mag::fs::json& data, const mag::math::vector3<T>& v)
    {
        data = mag::fs::json::array();

        for (u32 i = 0; i < 3; i++)
        {
            data.push_back(v[i]);
        }
    }

    template <typename T>
    inline void to_json(mag::fs::json& data, const mag::math::vector4<T>& v)
    {
        data = mag::fs::json::array();

        for (u32 i = 0; i < 4; i++)
        {
            data.push_back(v[i]);
        }
    }

    template <typename T>
    inline void from_json(const mag::fs::json& data, mag::math::vector2<T>& v)
    {
        if (!data.is_array() || data.size() != 2)
        {
            LOG_ERROR("Json and vec size mismatch");
            return;
        }

        for (u32 i = 0; i < 2; i++)
        {
            v[i] = data.at(i).get<T>();
        }
    }

    template <typename T>
    inline void from_json(const mag::fs::json& data, mag::math::vector3<T>& v)
    {
        if (!data.is_array() || data.size() != 3)
        {
            LOG_ERROR("Json and vec size mismatch");
            return;
        }

        for (u32 i = 0; i < 3; i++)
        {
            v[i] = data.at(i).get<T>();
        }
    }

    template <typename T>
    inline void from_json(const mag::fs::json& data, mag::math::vector4<T>& v)
    {
        if (!data.is_array() || data.size() != 4)
        {
            LOG_ERROR("Json and vec size mismatch");
            return;
        }

        for (u32 i = 0; i < 4; i++)
        {
            v[i] = data.at(i).get<T>();
        }
    }

    inline void to_json(mag::fs::json& data, const mag::math::quat& v) { data = {v[0], v[1], v[2], v[3]}; }

    inline void from_json(const mag::fs::json& data, mag::math::quat& v)
    {
        if (!data.is_array() || data.size() != 4)
        {
            LOG_ERROR("Json and quat size mismatch");
            return;
        }

        for (u32 i = 0; i < 4; i++)
        {
            v[i] = data[i].get<f32>();
        }
    }
};  // namespace nlohmann
