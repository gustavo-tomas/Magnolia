#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>
#include <string>
#include <vector>

namespace mag
{
    // Unsigned integers
    typedef uint8_t u8;
    typedef uint16_t u16;
    typedef uint32_t u32;
    typedef uint64_t u64;

    // Integers
    typedef int8_t i8;
    typedef int16_t i16;
    typedef int32_t i32;
    typedef int64_t i64;

    // Floats
    typedef float f32;
    typedef double f64;

    // Bool
    typedef bool b8;

    // Strings
    typedef std::string str;

    // Assert sizes
    static_assert(sizeof(u8) == 1, "Expected u8 to be 1 byte.");
    static_assert(sizeof(u16) == 2, "Expected u16 to be 2 bytes.");
    static_assert(sizeof(u32) == 4, "Expected u32 to be 4 bytes.");
    static_assert(sizeof(u64) == 8, "Expected u64 to be 8 bytes.");

    static_assert(sizeof(i8) == 1, "Expected i8 to be 1 byte.");
    static_assert(sizeof(i16) == 2, "Expected i16 to be 2 bytes.");
    static_assert(sizeof(i32) == 4, "Expected i32 to be 4 bytes.");
    static_assert(sizeof(i64) == 8, "Expected i64 to be 8 bytes.");

    static_assert(sizeof(f32) == 4, "Expected f32 to be 4 bytes.");
    static_assert(sizeof(f64) == 8, "Expected f64 to be 8 bytes.");

    static_assert(sizeof(b8) == 1, "Expected b8 to be 1 byte.");

// Common macros
#define VEC_SIZE_BYTES(vec) (vec.empty() ? 0 : vec.size() * sizeof(vec[0])) /* Vector size in bytes */
#define MAG_TIMEOUT 1'000'000'000                                           /* 1 second in nanoseconds */
#define BIND_FN(x) std::bind(&x, this, std::placeholders::_1)               /* Shortcut to bind methods */

    // Math definitions
    namespace math
    {
        // Wrapper for glm
        using namespace glm;

        const f32 Half_Pi = glm::half_pi<f32>();
        const f32 Pi = glm::pi<f32>();
        const f32 Two_Pi = glm::two_pi<f32>();

        // Simpler version of glm::decompose
        inline b8 decompose_simple(const mat4& model_matrix, vec3& scale, vec3& rotation, vec3& translation)
        {
            quat orientation;
            vec3 skew;
            vec4 perspective;

            const b8 result = glm::decompose(model_matrix, scale, orientation, translation, skew, perspective);
            rotation = degrees(glm::eulerAngles(orientation));

            return result;
        }

        // Axis Aligned Bounding Box. Also has a helper method to resize the bounding box after a transformation.
        struct BoundingBox
        {
                vec3 min;
                vec3 max;

                BoundingBox get_transformed_bounding_box(const mat4& transform) const
                {
                    BoundingBox transformed_aabb;

                    // Remove translation influence
                    mat4 model_without_transform = transform;
                    model_without_transform[3] = vec4(0.0f, 0.0f, 0.0f, 1.0f);

                    std::vector<vec3> vertices = {this->min,
                                                  vec3(this->min.x, this->min.y, this->max.z),
                                                  vec3(this->min.x, this->max.y, this->min.z),
                                                  vec3(this->min.x, this->max.y, this->max.z),
                                                  vec3(this->max.x, this->min.y, this->min.z),
                                                  vec3(this->max.x, this->min.y, this->max.z),
                                                  vec3(this->max.x, this->max.y, this->min.z),
                                                  this->max};

                    // Transform all vertices
                    for (auto& vertex : vertices)
                    {
                        vertex = model_without_transform * vec4(vertex, 1.0f);
                    }

                    // Recalculate min and max
                    transformed_aabb.min = vertices[0];
                    transformed_aabb.max = vertices[0];

                    for (const auto& vertex : vertices)
                    {
                        transformed_aabb.min = math::min(transformed_aabb.min, vertex);
                        transformed_aabb.max = math::max(transformed_aabb.max, vertex);
                    }

                    // Re-apply translation
                    transformed_aabb.min = translate(vec3(transform[3])) * vec4(transformed_aabb.min, 1.0f);
                    transformed_aabb.max = translate(vec3(transform[3])) * vec4(transformed_aabb.max, 1.0f);

                    return transformed_aabb;
                }
        };
    };  // namespace math
};      // namespace mag
