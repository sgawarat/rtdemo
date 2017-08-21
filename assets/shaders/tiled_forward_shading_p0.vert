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

layout(location = 10) uniform uint DRAW_ID; // HACK

void main() {
    vec4 position_c = CAMERA.view_proj * vec4(position, 1);
    gl_Position = position_c;
}
