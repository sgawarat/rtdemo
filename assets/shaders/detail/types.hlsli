// カメラ
struct Camera {
    float4x4 view_proj;
    float4x4 view;
    float4x4 proj;
    float4x4 view_proj_inv;
    float4x4 view_inv;
    float4x4 proj_inv;
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

struct GridCell {
    uint first;
    uint count;
};
