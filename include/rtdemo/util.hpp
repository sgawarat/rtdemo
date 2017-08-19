#pragma once

#include <string>
#include "garie.hpp"

namespace rtdemo {
namespace util {
garie::VertexShader compile_vertex_shader_from_file(
    const char* path, std::string* log_ptr = nullptr);
garie::FragmentShader compile_fragment_shader_from_file(
    const char* path, std::string* log_ptr = nullptr);
garie::ComputeShader compile_compute_shader_from_file(
    const char* path, std::string* log_ptr = nullptr);

garie::Program link_program(const garie::VertexShader& vert,
                            const garie::FragmentShader& frag,
                            std::string* log_ptr = nullptr);
garie::Program link_program(const garie::VertexShader& vert,
                            std::string* log_ptr = nullptr);
garie::Program link_program(const garie::ComputeShader& comp,
                            std::string* log_ptr = nullptr);

const garie::VertexArray& light_quad_vao();
void draw_light_quad();
const garie::VertexArray& screen_quad_vao();
void draw_screen_quad();

const garie::RasterizationState& discard_rs();
const garie::RasterizationState& default_rs();

const garie::ColorBlendState& default_bs();
const garie::ColorBlendState& alpha_blending_bs();
const garie::ColorBlendState& additive_bs();

const garie::DepthStencilState& default_dss();
const garie::DepthStencilState& depth_test_dss();
const garie::DepthStencilState& depth_test_no_write_dss();
}  // namespace util
}  // namespace rtdemo
