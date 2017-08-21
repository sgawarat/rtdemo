#version 450

layout(location = 0) in vec2 position;

out VertexData {
    layout(location = 0) vec2 texcoord;
} OUT;

void main() {
    OUT.texcoord = position * 0.5 + vec2(0.5);
    gl_Position = vec4(position, 0, 1);
}
