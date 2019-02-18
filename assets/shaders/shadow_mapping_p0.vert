#version 450

struct ShadowCaster {
    mat4 view_proj;
};

layout(binding = 0) buffer ShadowCasterBuffer {
    ShadowCaster SHADOW_CASTERS[];
};

layout(location = 0) in vec3 position;

void main() {
    vec4 position_c = SHADOW_CASTERS[0].view_proj * vec4(position, 1);

    gl_Position = position_c;
}
