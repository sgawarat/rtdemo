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

// 前段から受け取る値
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

// 次段に渡す値
out VertexData {
    layout(location = 0) vec3 position_w;
    layout(location = 1) vec3 normal_w;
} OUT;

void main() {
    // クリップ空間の位置を計算する
    vec4 position_c = CAMERA.view_proj * vec4(position, 1);

    // 出力する
    OUT.position_w = position;
    OUT.normal_w = normal;
    gl_Position = position_c;
}
