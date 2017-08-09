#pragma once

#include <cstdint>
#include <glm/glm.hpp>

namespace rtdemo {
namespace layout {
class StaticLayout final {
public:
    static const uint64_t ID;// = 1000;

    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
    };
    static constexpr int POSITION_ATTRIBUTE_LOCATION = 0;
    static constexpr int NORMAL_ATTRIBUTE_LOCATION = 1;

    using Index = uint16_t;

    struct Camera {
        glm::mat4 view_proj;
        glm::vec3 position_w;
        float _position_w;
    };
    static constexpr int CAMERA_UNIFORM_BINDING = 0;

    struct ResourceIndex {
        uint32_t material_index;
    };
    static constexpr int RESOURCE_INDEX_STORAGE_BINDING = 0;

    struct Material {
        glm::vec3 ambient;
        float _ambient;
        glm::vec3 diffuse;
        float _diffuse;
        glm::vec3 specular;
        float specular_power;
    };
    static constexpr int MATERIAL_STORAGE_BINDING = 1;

    struct Light {
        glm::vec3 position_w;
        float _position_w;
    };
    static constexpr int LIGHT_STORAGE_BINDING = 2;

    // same as gl_DrawID
    static constexpr int DRAW_ID_CONSTANT_LOCATION = 10;
};
}  // namespace layout
}  // namespace rtdemo
