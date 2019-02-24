#version 450

// カメラ
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

// 入力
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

// 出力
out VertexData {
    layout(location = 0) vec3 normal_w;
} OUT;

void main() {
    OUT.normal_w = normal;
    gl_Position = CAMERA.view_proj * vec4(position, 1);
}
