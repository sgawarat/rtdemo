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

layout(location = 11) uniform int DEBUG_VIEW;

void main() {
    vec2 texcoord = IN.position_c * 0.5 + vec2(0.5);

    float depth = texture(DEPTH, texcoord).r;
    vec4 g0 = texture(GBUFFER0, texcoord);
    vec4 g1 = texture(GBUFFER1, texcoord);
    vec4 g2 = texture(GBUFFER2, texcoord);
    vec4 g3 = texture(GBUFFER3, texcoord);
    vec3 normal_w = normalize(g0.rgb - vec3(0.5));
    vec3 ambient = g1.rgb;
    vec3 diffuse = g2.rgb;
    vec3 specular = g3.rgb;
    float specular_power = exp2(g3.a * 10.5);

    // this pixel has no samples
    if (depth == 1) discard;

    vec4 position_wh = CAMERA.view_proj_inv * vec4(IN.position_c, depth * 2 - 1, 1);
    vec3 position_w = position_wh.xyz / position_wh.w;

    vec3 final_color;
    if (DEBUG_VIEW == 1) {
        final_color = g0.rgb;
    } else if (DEBUG_VIEW == 2) {
        final_color = g1.rgb;
    } else if (DEBUG_VIEW == 3) {
        final_color = g2.rgb;
    } else if (DEBUG_VIEW == 4) {
        final_color = g3.rgb;
    } else if (DEBUG_VIEW == 5) {
        final_color = g3.aaa;
    } else if (DEBUG_VIEW == 6) {
        final_color = mod(abs(position_w), vec3(2)) / 2;
    } else {
        vec3 v = normalize(CAMERA.position_w - position_w);
        vec3 n = normalize(normal_w);

        final_color = vec3(0);

        int lights_length = LIGHTS.length();
        for (int i = 0; i < lights_length; ++i) {
            Light LIGHT = LIGHTS[i];

            vec3 l = normalize(LIGHT.position_w - position_w);
            vec3 r = reflect(-l, n);

            final_color += 
                //ambient
                //+
                diffuse * max(0, dot(n, l))
                +
                specular * pow(max(0, dot(v, r)), specular_power)
                ;
        }
    }

    frag_color = vec4(final_color, 1);
}
