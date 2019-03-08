
// タイルの大きさ
#define TILE_WIDTH 32
#define TILE_HEIGHT 32
#define TILE_SIZE (TILE_WIDTH * TILE_HEIGHT)

// タイル内で有効なライトの最大数
#define MAX_LIGHT_INDEX_COUNT 1024

// タイル
struct Tile {
    uint light_first;
    uint light_count;
    float min_depth;
    float max_depth;
};

// タイルの視錐台
struct TileFrustum {
    float4 planes[4];
};

// シーンの定数
cbuffer SceneConstant : register(b7) {
    uint LIGHT_COUNT;
};

// テクニックの定数
cbuffer TechConstant : register(b15) {
    uint2 TILE_COUNT;  // タイルの数
    uint2 DISPATCH_COUNT;  // ディスパッチの数
    int MODE;  // モード
};
