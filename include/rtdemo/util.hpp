#pragma once

#include <string>
#include "garie.hpp"

namespace rtdemo::util {
/**
 * @brief ファイルから頂点シェーダをコンパイルする
 * 
 * @param path ファイルパス
 * @param log_ptr ログを書き出す先へのポインタ
 * @return garie::VertexShader 生成したシェーダオブジェクト
 */
garie::VertexShader compile_vertex_shader_from_file(
    const char* path, std::string* log_ptr = nullptr);

/**
 * @brief ファイルからフラグメントシェーダをコンパイルする
 * 
 * @param path ファイルパス
 * @param log_ptr ログを書き出す先へのポインタ
 * @return garie::VertexShader 生成したシェーダオブジェクト
 */
garie::FragmentShader compile_fragment_shader_from_file(
    const char* path, std::string* log_ptr = nullptr);

/**
 * @brief ファイルからコンピュートシェーダをコンパイルする
 * 
 * @param path ファイルパス
 * @param log_ptr ログを書き出す先へのポインタ
 * @return garie::VertexShader 生成したシェーダオブジェクト
 */
garie::ComputeShader compile_compute_shader_from_file(
    const char* path, std::string* log_ptr = nullptr);

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
 * @brief 光源用のクアッドをバインドしたVAO
 * 
 * @return const garie::VertexArray& VAO
 */
const garie::VertexArray& light_quad_vao();

/**
 * @brief 光源用のクアッドを描画する
 * 
 */
void draw_light_quad();

/**
 * @brief スクリーンクアッドをバインドしたVAO
 * 
 * @return const garie::VertexArray& VAO
 */
const garie::VertexArray& screen_quad_vao();

/**
 * @brief スクリーンクアッドをバインドしたVAO
 * 
 */
void draw_screen_quad();

/**
 * @brief ラスタライザを無効化するステート
 * 
 * @return const garie::RasterizationState& ステートを返す
 */
const garie::RasterizationState& discard_rs();

/**
 * @brief 既定値のステート
 * 
 * @return const garie::RasterizationState& ステートを返す
 */
const garie::RasterizationState& default_rs();

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
}  // namespace rtdemo::util
