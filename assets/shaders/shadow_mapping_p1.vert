#version 450

struct Camera {
    mat4 view_proj;
    mat4 view;
    mat4 proj;
    mat4 view_proj_inv;
    mat4 view_inv;
    mat4 proj_inv;
    vec3 position_w;
};
struct ShadowCaster {
    mat4 view_proj;
};

layout(binding = 0) uniform CameraUniform {
    Camera CAMERA;
};
layout(binding = 3) buffer ShadowCasterBuffer {
    ShadowCaster SHADOW_CASTERS[];
};

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

out VertexData {
    layout(location = 0) vec3 position_w;
    layout(location = 1) vec3 normal_w;
} OUT;

layout(location = 32) uniform uint DEBUG_VIEW;

void main() {
    vec4 position_c;
    
    if (DEBUG_VIEW == 2) {  // CASTER
        position_c = SHADOW_CASTERS[0].view_proj * vec4(position, 1);
    } else {
        position_c = CAMERA.view_proj * vec4(position, 1);
    }

    OUT.position_w = position;
    OUT.normal_w = normal;
    gl_Position = position_c;
}
