#pragma once

#include "magnolia/core/logger.hpp"
#include "magnolia/math/types.hpp"
#include "magnolia/platform/file_system.hpp"
#include "nlohmann/json.hpp"

namespace nlohmann
{
    template <i32 L, typename T, mag::math::qualifier Q>
    inline void to_json(mag::fs::json& data, const mag::math::vec<L, T, Q>& v)
    {
        data = mag::fs::json::array();

        for (i32 i = 0; i < L; ++i)
        {
            data.push_back(v[i]);
        }
    }

    template <i32 L, typename T, mag::math::qualifier Q>
    inline void from_json(const mag::fs::json& data, mag::math::vec<L, T, Q>& v)
    {
        if (!data.is_array() || data.size() != L)
        {
            LOG_ERROR("Json and vec size mismatch");
            return;
        }

        for (i32 i = 0; i < L; ++i)
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

        for (i32 i = 0; i < mag::math::quat::length(); i++)
        {
            v[i] = data[i].get<f32>();
        }
    }
};  // namespace nlohmann
