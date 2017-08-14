#version 450

layout(binding = 0) uniform Camera {
    mat4 view_proj;
    mat4 view_proj_inv;
    vec3 position_w;
} CAMERA;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

out VertexData {
    layout(location = 0) vec3 position_w;
    layout(location = 1) vec3 normal_w;
} OUT;

void main() {
    vec4 position_c = CAMERA.view_proj * vec4(position, 1);

    OUT.position_w = position;
    OUT.normal_w = normal;
    gl_Position = position_c;
}
