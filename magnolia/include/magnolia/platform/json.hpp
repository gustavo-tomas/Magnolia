#pragma once

#include "magnolia/core/logger.hpp"
#include "magnolia/math/types.hpp"
#include "magnolia/platform/file_system.hpp"
#include "nlohmann/json.hpp"

namespace nlohmann
{
    // @TODO: to_json is not working, so we use these operators for now

    inline mag::fs::json& operator<<(mag::fs::json& out, const mag::math::vec2& v)
    {
        for (i32 i = 0; i < mag::math::vec2::length(); i++)
        {
            out.push_back(v[i]);
        }
        return out;
    }

    inline mag::fs::json& operator<<(mag::fs::json& out, const mag::math::vec3& v)
    {
        for (i32 i = 0; i < mag::math::vec3::length(); i++)
        {
            out.push_back(v[i]);
        }
        return out;
    }

    inline mag::fs::json& operator<<(mag::fs::json& out, const mag::math::vec4& v)
    {
        for (i32 i = 0; i < mag::math::vec4::length(); i++)
        {
            out.push_back(v[i]);
        }
        return out;
    }

    inline mag::fs::json& operator<<(mag::fs::json& out, const mag::math::quat& q)
    {
        for (i32 i = 0; i < mag::math::quat::length(); i++)
        {
            out.push_back(q[i]);
        }
        return out;
    }

    template <i32 L, typename T, glm::qualifier Q>
    struct adl_serializer<glm::vec<L, T, Q>>
    {
            static void to_json(json& data, const glm::vec<L, T, Q>& v)
            {
                data = json::array();

                for (i32 i = 0; i < L; ++i)
                {
                    data.push_back(v[i]);
                }
            }

            static void from_json(const json& data, glm::vec<L, T, Q>& v)
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
    };

    template <>
    struct adl_serializer<mag::math::quat>
    {
            static void to_json(json& data, const mag::math::quat& v) { data = {v[0], v[1], v[2], v[3]}; }

            static void from_json(const json& data, mag::math::quat& v)
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
    };
};  // namespace nlohmann
