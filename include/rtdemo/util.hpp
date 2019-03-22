#pragma once

#include <string>
#include <filesystem>
#include "garie.hpp"

namespace rtdemo::util {
/**
 * @brief シェーダファイルを探すディレクトリを指定する
 * 
 * @param path ディレクトリへのパス
 */
void set_shader_search_path(std::filesystem::path path);

/**
 * @brief ファイルから頂点シェーダをコンパイルする
 * 
 * @param path ファイルパス
 * @param log_ptr ログを書き出す先へのポインタ
 * @return garie::VertexShader 生成したシェーダオブジェクト
 */
garie::VertexShader compile_vertex_shader_from_file(
    std::filesystem::path path, std::string* log_ptr = nullptr);

/**
 * @brief ファイルからフラグメントシェーダをコンパイルする
 * 
 * @param path ファイルパス
 * @param log_ptr ログを書き出す先へのポインタ
 * @return garie::VertexShader 生成したシェーダオブジェクト
 */
garie::FragmentShader compile_fragment_shader_from_file(
    std::filesystem::path path, std::string* log_ptr = nullptr);

/**
 * @brief ファイルからコンピュートシェーダをコンパイルする
 * 
 * @param path ファイルパス
 * @param log_ptr ログを書き出す先へのポインタ
 * @return garie::VertexShader 生成したシェーダオブジェクト
 */
garie::ComputeShader compile_compute_shader_from_file(
    std::filesystem::path path, std::string* log_ptr = nullptr);

/**
 * @brief ファイルから頂点シェーダをロードする
 * 
 * @param path ファイルパス
 * @param log_ptr ログを書き出す先へのポインタ
 * @return garie::VertexShader 生成したシェーダオブジェクト
 */
garie::VertexShader load_vertex_shader_from_file(
    std::filesystem::path path, std::string* log_ptr = nullptr);

/**
 * @brief ファイルからフラグメントシェーダをロードする
 * 
 * @param path ファイルパス
 * @param log_ptr ログを書き出す先へのポインタ
 * @return garie::VertexShader 生成したシェーダオブジェクト
 */
garie::FragmentShader load_fragment_shader_from_file(
    std::filesystem::path path, std::string* log_ptr = nullptr);

/**
 * @brief ファイルからコンピュートシェーダをロードする
 * 
 * @param path ファイルパス
 * @param log_ptr ログを書き出す先へのポインタ
 * @return garie::VertexShader 生成したシェーダオブジェクト
 */
garie::ComputeShader load_compute_shader_from_file(
    std::filesystem::path path, std::string* log_ptr = nullptr);


/**
 * @brief プログラムをリンクする
 * 
 * @param vert 頂点シェーダ
 * @param frag フラグメントシェーダ
 * @param log_ptr ログを書き出す先へのポインタ
 * @return garie::Program 生成したシェーダプログラム
 */
garie::Program link_program(const garie::VertexShader& vert,
                            const garie::FragmentShader& frag,
                            std::string* log_ptr = nullptr);

/**
 * @brief プログラムをリンクする
 * 
 * @param vert 頂点シェーダ
 * @param log_ptr ログを書き出す先へのポインタ
 * @return garie::Program 生成したシェーダプログラム
 */
garie::Program link_program(const garie::VertexShader& vert,
                            std::string* log_ptr = nullptr);

/**
 * @brief プログラムをリンクする
 * 
 * @param comp コンピュートシェーダ
 * @param log_ptr ログを書き出す先へのポインタ
 * @return garie::Program 生成したシェーダプログラム
 */
garie::Program link_program(const garie::ComputeShader& comp,
                            std::string* log_ptr = nullptr);

/**
 * @brief スクリーン全体にまたがる四角形をバインドしたVAOを取得する
 * 
 * @return const garie::VertexArray& VAO
 */
const garie::VertexArray& screen_quad_vao();

/**
 * @brief スクリーン全体にまたがる四角形を描画する
 * 
 */
void draw_screen_quad();

/**
 * @brief スクリーン全体にまたがる三角形をバインドしたVAOを取得する
 * 
 * @return const garie::VertexArray& VAO
 */
const garie::VertexArray& screen_triangle_vao();

/**
 * @brief スクリーン全体にまたがる三角形を描画する
 */
void draw_screen_quad();

/**
 * @brief 既定値のステート
 * 
 * @return const garie::RasterizationState& ステートを返す
 */
const garie::RasterizationState& default_rs();

/**
 * @brief ラスタライザを無効化するステート
 * 
 * @return const garie::RasterizationState& ステートを返す
 */
const garie::RasterizationState& discard_rs();

/**
 * @brief 背面を描画するステート
 * 
 * @return const garie::RasterizationState& ステートを返す
 */
const garie::RasterizationState& backface_rs();

/**
 * @brief 既定値のステート
 * 
 * @return const garie::ColorBlendState& ステート
 */
const garie::ColorBlendState& default_bs();

/**
 * @brief アルファブレンディングのステート
 * 
 * @return const garie::ColorBlendState& ステート
 */
const garie::ColorBlendState& alpha_blending_bs();

/**
 * @brief 加算ブレンディングのステート
 * 
 * @return const garie::ColorBlendState& ステート
 */
const garie::ColorBlendState& additive_bs();

/**
 * @brief 既定値のステート
 * 
 * @return const garie::DepthStencilState& ステート
 */
const garie::DepthStencilState& default_dss();

/**
 * @brief 深度テストと深度書き込みを有効化したステート
 * 
 * @return const garie::DepthStencilState& ステート
 */
const garie::DepthStencilState& depth_test_dss();

/**
 * @brief 深度テストのみ有効化したステート
 * 
 * @return const garie::DepthStencilState& ステート
 */
const garie::DepthStencilState& depth_test_no_write_dss();

/**
 * @brief 色をクリアする
 */
void clear(std::array<float, 4> color);

/**
 * @brief 深度をクリアする
 */
void clear(float depth);

/**
 * @brief 色と深度をクリアする
 */
void clear(std::array<float, 4> color, float depth);

/**
 * @brief 色と深度とステンシルをクリアする
 */
void clear(std::array<float, 4> color, float depth, GLint stencil);

/**
 * @brief スクリーンサイズのビューポートを生成する
 * 
 * @return Viewport 
 */
garie::Viewport screen_viewport();

/**
 * @brief スクリーンサイズのScissorを生成する
 * 
 * @return Scissor 
 */
garie::Scissor screen_scissor();
}  // namespace rtdemo::util
