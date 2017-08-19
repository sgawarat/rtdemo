#version 450

layout(binding = 0) uniform Camera {
    mat4 view_proj;
    mat4 view;
    mat4 proj;
    mat4 view_proj_inv;
    vec3 position_w;
} CAMERA;

layout(location = 0) in vec3 position;

void main() {
    vec4 position_c = CAMERA.view_proj * vec4(position, 1);
    gl_Position = position_c;
}
