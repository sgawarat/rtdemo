#version 450
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float specular_power;
};
struct Light {
    vec3 position_w;
};

layout(binding = 0) uniform Camera {
    mat4 view_proj;
    mat4 view_proj_inv;
    vec3 position_w;
} CAMERA;
layout(binding = 0) buffer ResourceIndexBuffer {
    uint RESOURCE_INDICES[];
};
layout(binding = 1) buffer MaterialBuffer {
    Material MATERIALS[];
};
layout(binding = 2) buffer LightBuffer {
    Light LIGHTS[];
};

layout(binding = 0) uniform sampler2D DEPTH;
layout(binding = 1) uniform sampler2D GBUFFER0;
layout(binding = 2) uniform sampler2D GBUFFER1;
layout(binding = 3) uniform sampler2D GBUFFER2;
layout(binding = 4) uniform sampler2D GBUFFER3;

in VertexData {
    layout(location = 0) vec2 position_c;
} IN;

layout(location = 0) out vec4 frag_color;

void main() {
    vec2 texcoord = IN.position_c * 0.5 + vec2(0.5);

    float depth = texture(DEPTH, texcoord).r;
    vec3 normal_w = texture(GBUFFER0, texcoord).rgb * 2 - vec3(1);
    vec3 ambient = texture(GBUFFER1, texcoord).rgb;
    vec3 diffuse = texture(GBUFFER2, texcoord).rgb;
    vec4 g3 = texture(GBUFFER3, texcoord).rgba;
    vec3 specular = g3.rgb;
    float specular_power = g3.a;

    vec4 position_wh = CAMERA.view_proj_inv * vec4(IN.position_c, depth, 1);
    vec3 position_w = position_wh.xyz / position_wh.w;

    Light LIGHT = LIGHTS[0];

    vec3 l = normalize(LIGHT.position_w - position_w);
    vec3 v = normalize(CAMERA.position_w - position_w);
    vec3 n = normalize(normal_w);
    vec3 r = reflect(-l, n);

    vec3 final_color =
        //ambient
        //+
        diffuse * max(0, dot(n, l))
        //+
        //specular * pow(max(0, dot(v, r)), specular_power)
        ;
    frag_color = vec4(final_color, 1);
}
