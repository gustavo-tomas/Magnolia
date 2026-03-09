#pragma once

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

    template <>
    struct adl_serializer<mag::math::vec2>
    {
            static void from_json(const json& data, mag::math::vec2& v)
            {
                for (i32 i = 0; i < mag::math::vec2::length(); i++)
                {
                    v[i] = data[i].get<f32>();
                }
            }

            static void to_json(json& data, const mag::math::vec2& v) { data = json{v[0], v[1]}; }
    };

    template <>
    struct adl_serializer<mag::math::vec3>
    {
            static void from_json(const json& data, mag::math::vec3& v)
            {
                for (i32 i = 0; i < mag::math::vec3::length(); i++)
                {
                    v[i] = data[i].get<f32>();
                }
            }

            static void to_json(json& data, const mag::math::vec3& v) { data = {v[0], v[1], v[2]}; }
    };

    template <>
    struct adl_serializer<mag::math::vec4>
    {
            static void from_json(const json& data, mag::math::vec4& v)
            {
                for (i32 i = 0; i < mag::math::vec4::length(); i++)
                {
                    v[i] = data[i].get<f32>();
                }
            }

            static void to_json(json& data, const mag::math::vec4& v) { data = {v[0], v[1], v[2], v[3]}; }
    };

    template <>
    struct adl_serializer<mag::math::quat>
    {
            static void from_json(const json& data, mag::math::quat& v)
            {
                for (i32 i = 0; i < mag::math::quat::length(); i++)
                {
                    v[i] = data[i].get<f32>();
                }
            }

            static void to_json(json& data, const mag::math::quat& v) { data = {v[0], v[1], v[2], v[3]}; }
    };
};  // namespace nlohmann
