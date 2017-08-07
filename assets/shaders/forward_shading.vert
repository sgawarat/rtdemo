#version 450
// #extension GL_ARB_shader_draw_parameters : require
layout(binding = 0) uniform Camera {
    mat4 wvp;
    vec3 position_w;
} CAMERA;
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
out VertexData {
    layout(location = 0) vec3 position_w;
    layout(location = 1) vec3 normal_w;
    // layout(location = 2) uint draw_id;
} OUT;
void main() {
    OUT.position_w = position;
    OUT.normal_w = normal;
    // OUT.draw_id = IN.gl_DrawID;
    gl_Position = CAMERA.wvp * vec4(position, 1);
}
