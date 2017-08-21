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
struct PointLight {
    vec3 position_w;
    float radius;
    vec3 color;
    float intensity;
};

layout(binding = 0) uniform CameraUniform {
    Camera CAMERA;
};

layout(binding = 0) buffer LightBuffer {
    PointLight LIGHTS[];
};

layout(location = 0) in vec2 position;

out VertexData {
    layout(location = 0) vec3 position_c;
} OUT;

layout(location = 10) uniform int DRAW_ID;

void main() {
    PointLight LIGHT = LIGHTS[DRAW_ID];

    // vec4 light_position_v = CAMERA.view * vec4(LIGHT.position_w, 1);
    // vec3 position_v = light_position_v.xyz + vec3(position, 0) * LIGHT.radius;
    // vec4 position_c = CAMERA.proj * vec4(position_v, 1);
    vec4 position_c = vec4(position, 0, 1);

    OUT.position_c = position_c.xyz / position_c.w;
    gl_Position = position_c;
}
