
// タイルの大きさ
#define TILE_WIDTH 32
#define TILE_HEIGHT 32
#define TILE_SIZE (TILE_WIDTH * TILE_HEIGHT)

// タイル内で有効なライトの最大数
#define MAX_LIGHT_INDEX_COUNT 1024

// タイル
struct Tile {
    uint light_index_first;  // ライトインデックスリストのオフセット
    uint light_index_count;  // ライトインデックスの数
};

// タイルの視錐台
struct TileFrustum {
    float4 planes[4];
};

// シーンの定数
cbuffer SceneConstant : register(b7) {
    uint LIGHT_COUNT;  // ライトの数
};

// テクニックの定数
cbuffer TechConstant : register(b15) {
    uint2 TILE_COUNT;  // スクリーンを占めるタイルの数
    uint2 PIXEL_COUNT;  // スクリーンを占めるピクセルの数
    int MODE;  // 表示するモード
};
