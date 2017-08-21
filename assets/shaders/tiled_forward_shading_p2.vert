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

layout(binding = 0) uniform CameraUniform {
    Camera CAMERA;
};

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

out VertexData {
    layout(location = 0) vec3 position_w;
    layout(location = 1) vec3 normal_w;
    layout(location = 2) vec2 position_s;
} OUT;

void main() {
    vec4 position_c = CAMERA.view_proj * vec4(position, 1);
    vec2 position_s = clamp((position_c.xy / position_c.w) * 0.5 + 0.5, vec2(0), vec2(0.999999));

    OUT.position_w = position;
    OUT.normal_w = normal;
    OUT.position_s = position_s.xy;
    gl_Position = position_c;
}
