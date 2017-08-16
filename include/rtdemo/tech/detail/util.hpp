#pragma once

#include <fstream>
#include <vector>
#include <string>
#include <rtdemo/garie.hpp>
#include <rtdemo/logging.hpp>

namespace rtdemo {
namespace tech {
namespace detail {
template <GLenum TYPE>
garie::Shader<TYPE> compile_shader_from_file(const char* path,
                                             std::string* log_ptr = nullptr);
// template
// garie::VertexShader compile_shader_from_file(const char*, std::string*);
// template
// garie::FragmentShader compile_shader_from_file(const char*, std::string*);
                                             
garie::Program link_program(const garie::VertexShader& vert,
                            const garie::FragmentShader& frag,
                            std::string* log_ptr = nullptr);

// ScreenQuad geometry for common use
const garie::VertexArray& screen_quad_vao();
void draw_screen_quad();

const garie::RasterizationState& default_rs();

const garie::ColorBlendState& default_bs();
const garie::ColorBlendState& alpha_blending_bs();
const garie::ColorBlendState& additive_bs();

const garie::DepthStencilState& default_dss();
const garie::DepthStencilState& depth_test_dss();
}  // namespace detail
}  // namespace tech
}  // namespace rtdemo
