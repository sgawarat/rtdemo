#version 450

in VertexData {
    layout(location = 0) vec2 texcoord;
} IN;

layout(location = 0) out vec4 frag_color;

layout(binding = 8) uniform sampler2D RT0;

void main() {
    frag_color = texelFetch(RT0, ivec2(IN.texcoord * textureSize(RT0, 0)), 0);
}
