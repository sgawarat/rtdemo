#include <common.hlsli>

// 入力
struct PSInput {
    [[vk::location(0)]] float3 position_ndc : POSITION_NDC;
    // [[vk::location(1)]] uint draw_id : DRAW_ID;
};

// 出力
struct PSOutput {
    [[vk::location(0)]] float4 frag_color : SV_Target;
};

// デバッグ用
struct Debug {
    int mode;
};

// b
[[vk::binding(0)]] ConstantBuffer<Camera> CAMERA : register(b0);
[[vk::binding(8)]] ConstantBuffer<Debug> DEBUG : register(b8);

// t
[[vk::binding(0)]] StructuredBuffer<PointLight> LIGHTS : register(t0);
[[vk::binding(8)]] Texture2D<float> DEPTH : register(t8);
[[vk::binding(9)]] Texture2D<float4> GBUFFER0 : register(t9);
[[vk::binding(10)]] Texture2D<float4> GBUFFER1 : register(t10);
[[vk::binding(11)]] Texture2D<float4> GBUFFER2 : register(t11);
[[vk::binding(12)]] Texture2D<float4> GBUFFER3 : register(t12);
SamplerState SAMPLER : register(s8);

void main(in PSInput i, out PSOutput o) {
    float2 texcoord = i.position_ndc.xy * 0.5f + float2(0.5f, 0.5f);

    float depth = DEPTH.Sample(SAMPLER, texcoord);
    if (depth == 1) discard;  // this pixel has no samples

    float4 g0 = GBUFFER0.Sample(SAMPLER, texcoord);
    float4 g1 = GBUFFER1.Sample(SAMPLER, texcoord);
    float4 g2 = GBUFFER2.Sample(SAMPLER, texcoord);
    float4 g3 = GBUFFER3.Sample(SAMPLER, texcoord);
    float3 normal_w = normalize(g0.rgb * 2 - float3(1.f, 1.f, 1.f));
    float3 ambient = g1.rgb;
    float3 diffuse = g2.rgb;
    float3 specular = g3.rgb;
    float specular_power = exp2(g3.a * 10.5f);

    float4 position_wh = mul(float4(i.position_ndc.xy, depth * 2.f - 1.f, 1.f), CAMERA.view_proj_inv);
    float3 position_w = position_wh.xyz / position_wh.w;

    float3 final_color;
    if (DEBUG.mode == 1) {
        final_color = float3(depth, depth, depth);
    } else if (DEBUG.mode == 2) {
        final_color = g0.rgb;
    } else if (DEBUG.mode == 3) {
        final_color = g1.rgb;
    } else if (DEBUG.mode == 4) {
        final_color = g2.rgb;
    } else if (DEBUG.mode == 5) {
        final_color = g3.rgb;
    } else if (DEBUG.mode == 6) {
        final_color = g3.aaa;
    } else if (DEBUG.mode == 7) {
        final_color = abs(position_w) % float3(1.f, 1.f, 1.f);
    } else {
        float3 v = normalize(CAMERA.position_w - position_w);
        float3 n = normalize(normal_w);

        PointLight LIGHT = LIGHTS[G.draw_id];

        float3 lv = LIGHT.position_w - position_w;
        float l_len = length(lv);
        float atten = 1.f;
        if (l_len >= LIGHT.radius) atten = 0.f;
        float3 l = (LIGHT.position_w - position_w) / l_len;
        float3 r = reflect(-l, n);

        final_color = 
            //ambient
            //+
            (diffuse * max(0.f, dot(n, l))
            +
            specular * pow(max(0.f, dot(v, r)), specular_power)
            ) * LIGHT.color * LIGHT.intensity * atten;
    }

    o.frag_color = float4(final_color, 1.f);
}
