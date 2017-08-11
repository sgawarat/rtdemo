#pragma once

#include <cstdint>
#include <glm/glm.hpp>

namespace rtdemo {
namespace layout {
class StaticLayout final {
public:
    static const uint64_t ID;// = 1000;

    // draw pass
    struct DrawPass final {
        static constexpr size_t GEOMETRIES = 0;
    };

    // input
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
    };
    using Index = uint16_t;
    struct AttributeLocation final {
        static constexpr int POSITION = 0;
        static constexpr int NORMAL = 1;
    };

    // constant
    struct ConstantLocation final {
        static constexpr int DRAW_ID = 10;  // same as gl_DrawID
    };
    
    // uniform buffer
    struct Camera {
        glm::mat4 view_proj;
        glm::vec3 position_w;
        float _position_w;
    };
    struct UniformBinding final {
        static constexpr int CAMERA = 0;
    };

    // storage buffer
    struct ResourceIndex {
        uint32_t material_index;
    };
    struct Material {
        glm::vec3 ambient;
        float _ambient;
        glm::vec3 diffuse;
        float _diffuse;
        glm::vec3 specular;
        float specular_power;
    };
    struct Light {
        glm::vec3 position_w;
        float _position_w;
    };
    struct StorageBinding final {
        static constexpr int RESOURCE_INDEX = 0;
        static constexpr int MATERIAL = 1;
        static constexpr int LIGHT = 2;
    };
};
}  // namespace layout
}  // namespace rtdemo
