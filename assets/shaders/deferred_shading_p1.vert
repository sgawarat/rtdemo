#version 450

layout(location = 0) in vec2 position;

out VertexData {
    layout(location = 0) vec2 position_c;
} OUT;

void main() {
    OUT.position_c = position;
    gl_Position = vec4(position, 0, 1);
}
