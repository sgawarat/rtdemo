// カメラ
struct Camera {
    float4x4 view_proj;
    float4x4 view;
    float4x4 proj;
    float4x4 view_proj_inv;
    float4x4 view_inv;
    float4x4 proj_inv;
    float4 range;  // x:幅、y:高さ、z:ニア面、w:ファー面
    float3 position_w;
};

// マテリアル
struct Material {
    float3 ambient;
    float3 diffuse;
    float3 specular;
    float specular_power;
};

// 点光源
struct PointLight {
    float3 position_w;
    float radius;
    float3 color;
    float intensity;
};

// シャドウキャスタ
struct ShadowCaster {
    float4x4 view_proj;
};

// 球
struct Sphere {
    float3 c;  // 中心点の位置
    float r;  // 半径
};

// 平面
struct Plane {
    float3 n;  // 法線
    float d;  // 原点からの距離
};

Plane new_plane(float3 n, float d) {
    Plane plane = {n, d};
    return plane;
}

// push constant
struct PushConstant {
    uint draw_id;
};
[[vk::push_constant]] PushConstant G;

#define M_EPSILON 1e-6

#define M_PI   3.14159265358979
#define M_2PI  6.28318530717959
#define M_4PI 12.56637061435917
