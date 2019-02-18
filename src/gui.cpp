#include <rtdemo/gui.hpp>
#ifdef WIN32
#include <Windows.h>
#endif
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <imgui.h>
#include <rtdemo/logging.hpp>

namespace rtdemo::gui {
namespace {
constexpr size_t INDICES_SIZE = 6 * 1024 * sizeof(ImDrawIdx);  // インデックスバッファのサイズ
constexpr size_t VERTICES_SIZE = 4 * 1024 * sizeof(ImDrawVert);  // 頂点バッファのサイズ
}  // namespace

bool Gui::init(GLFWwindow* window) {
  // コンテキストを生成する
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  // スタイルを設定する
  ImGui::StyleColorsDark();
  // ImGui::StyleColorsLight();

  // 初期化
  ImGuiIO& io = ImGui::GetIO();
  io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;  // マウスカーソルの位置を取得できる
  io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;  // マウスカーソルの位置を変更できる

  // キーマッピング
  io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
  io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
  io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
  io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
  io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
  io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
  io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
  io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
  io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
  io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
  io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
  io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
  io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
  io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
  io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
  io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
  io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
  io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
  io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

  // クリップボードのやり取りを行なうコールバックを設定する
  io.SetClipboardTextFn = [](void* user_data, const char* text) {
    glfwSetClipboardString(reinterpret_cast<GLFWwindow*>(user_data), text);
  };
  io.GetClipboardTextFn = [](void* user_data) {
    return glfwGetClipboardString(reinterpret_cast<GLFWwindow*>(user_data));
  };
  io.ClipboardUserData = window;

#ifdef WIN32
  // IMEの状態を監視するウィンドウのハンドル
  io.ImeWindowHandle = glfwGetWin32Window(window);
#endif

  // シェーダを生成する
  const GLchar* vert_code = R"CODE(
        #version 450
        layout(location = 0) uniform mat4 PROJ;
        layout(location = 0) in vec2 position;
        layout(location = 1) in vec2 texcoord;
        layout(location = 2) in vec4 color;
        out VertexData {
            layout(location = 0) vec2 texcoord;
            layout(location = 1) vec4 color;
        } OUT;
        void main() {
            OUT.texcoord = texcoord;
            OUT.color = color;
            gl_Position = PROJ * vec4(position, 0, 1);
        }
    )CODE";
  garie::VertexShader vert;
  vert.gen();
  if (!vert.compile(vert_code)) {
    GLchar info_log[1024];
    vert.get_info_log(1024, info_log);
    RT_LOG(error, "GUIの頂点シェーダのコンパイルに失敗した。(info_log:{})",
           info_log);
    return false;
  }

  const GLchar* frag_code = R"CODE(
        #version 450
        layout(binding = 0) uniform sampler2D FONT_TEX;
        in VertexData {
            layout(location = 0) vec2 texcoord;
            layout(location = 1) vec4 color;
        } IN;
        layout(location = 0) out vec4 frag_color;
        void main() {
            float font_color = texture(FONT_TEX, IN.texcoord).r;
            vec3 final_color = pow(IN.color.rgb, vec3(2.2));
            frag_color = vec4(final_color, IN.color.a * font_color);
        }
    )CODE";
  garie::FragmentShader frag;
  frag.gen();
  if (!frag.compile(frag_code)) {
    GLchar info_log[1024];
    frag.get_info_log(1024, info_log);
    RT_LOG(error, "GUIのフラグメントシェーダのコンパイルに失敗した。(info_log:{})",
           info_log);
    return false;
  }

  // シェーダプログラムを生成する
  garie::Program prog;
  prog.gen();
  if (!prog.link(vert, frag)) {
    GLchar info_log[1024];
    prog.get_info_log(1024, info_log);
    RT_LOG(error, "GUIのシェーダプログラムのリンクに失敗した。(info_log:{})",
           info_log);
    return false;
  }

  // ステートを生成する
  auto rs =
      garie::RasterizationStateBuilder().cull_mode(GL_NONE).build();
  auto cbs =
      garie::ColorBlendStateBuilder()
          .enable(0, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD,
                  GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD,
                  {GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE})
          .build();
  auto dss = garie::DepthStencilStateBuilder().build();

  // インデックスバッファを生成する
  garie::Buffer ib;
  ib.gen();
  ib.bind(GL_ELEMENT_ARRAY_BUFFER);
  glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, INDICES_SIZE, nullptr,
                  GL_MAP_WRITE_BIT);

  // 頂点バッファを生成する
  garie::Buffer vb;
  vb.gen();
  vb.bind(GL_ARRAY_BUFFER);
  glBufferStorage(GL_ARRAY_BUFFER, VERTICES_SIZE, nullptr, GL_MAP_WRITE_BIT);

  // VAOを生成する
  auto va =
      garie::VertexArrayBuilder()
          .index_buffer(ib)
          .vertex_buffer(vb)
          .attribute(0, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert),
                     offsetof(ImDrawVert, pos), 0)
          .attribute(1, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert),
                     offsetof(ImDrawVert, uv), 0)
          .attribute(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert),
                     offsetof(ImDrawVert, col), 0)
          .build();

  // フォントのイメージを格納するテクスチャを生成する
  unsigned char* pixels;
  int width, height;
  io.Fonts->AddFontFromFileTTF("assets/fonts/migu-1m-regular.ttf", 14.f,
                               nullptr, io.Fonts->GetGlyphRangesJapanese());
  io.Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);
  garie::Texture font_tex;
  font_tex.gen();
  font_tex.bind(GL_TEXTURE_2D);
  glTexStorage2D(GL_TEXTURE_2D, 1, GL_R8, width, height);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED,
                  GL_UNSIGNED_BYTE, pixels);
  // glGenerateMipmap(GL_TEXTURE_2D);
  io.Fonts->TexID = reinterpret_cast<void*>(static_cast<intptr_t>(font_tex_.id()));
  io.Fonts->ClearTexData();

  // サンプラを生成する
  garie::Sampler font_ss = garie::SamplerBuilder()
                               .min_filter(GL_NEAREST)
                               .mag_filter(GL_NEAREST)
                               .build();

  // 後始末
  window_ = window;
  prog_ = std::move(prog);
  rs_ = std::move(rs);
  cbs_ = std::move(cbs);
  dss_ = std::move(dss);
  va_ = std::move(va);
  ib_ = std::move(ib);
  vb_ = std::move(vb);
  font_tex_ = std::move(font_tex);
  font_ss_ = std::move(font_ss);

  return true;
}

void Gui::terminate() {
  prog_ = garie::Program();
  va_ = garie::VertexArray();
  ib_ = garie::Buffer();
  vb_ = garie::Buffer();
  if (font_tex_) {
    ImGui::GetIO().Fonts->TexID = 0;
    font_tex_ = garie::Texture();
  }
  ImGui::DestroyContext();
}

void Gui::new_frame() {
  ImGuiIO& io = ImGui::GetIO();

  // ディスプレイサイズを設定する
  int w, h;
  int display_w, display_h;
  glfwGetWindowSize(window_, &w, &h);
  glfwGetFramebufferSize(window_, &display_w, &display_h);
  io.DisplaySize = ImVec2((float)w, (float)h);
  io.DisplayFramebufferScale = ImVec2(w > 0 ? ((float)display_w / w) : 0,
                                      h > 0 ? ((float)display_h / h) : 0);

  // 時間間隔を設定する
  double current_time = glfwGetTime();
  io.DeltaTime =
      time_ > 0.0 ? (float)(current_time - time_) : (float)(1.0f / 60.0f);
  time_ = current_time;

  // マウスカーソルの位置を更新する
  const ImVec2 mouse_pos = io.MousePos;
  io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
  if (glfwGetWindowAttrib(window_, GLFW_FOCUSED)) {
    if (io.WantSetMousePos) {
      glfwSetCursorPos(window_, static_cast<double>(mouse_pos.x), static_cast<double>(mouse_pos.y));
    } else {
      double mouse_x, mouse_y;
      glfwGetCursorPos(window_, &mouse_x, &mouse_y);
      io.MousePos = ImVec2(static_cast<float>(mouse_x), static_cast<float>(mouse_y));
    }
  }

  // マウスボタンの状態を更新する
  for (int i = 0; i < 3; i++) {
    io.MouseDown[i] = mouse_pressed_[i] ||
                      glfwGetMouseButton(window_, i);
    mouse_pressed_[i] = false;
  }

  // フレーム開始
  ImGui::NewFrame();
}

void Gui::render() {
  ImGui::Render();

  ImDrawData* draw_data = ImGui::GetDrawData();

  ImGuiIO& io = ImGui::GetIO();

  // 
  int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
  int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
  if (fb_width == 0 || fb_height == 0) return;
  draw_data->ScaleClipRects(io.DisplayFramebufferScale);

  glEnable(GL_SCISSOR_TEST);
  glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
  const float ortho_projection[16] = {
      2.0f / io.DisplaySize.x,
      0.0f,
      0.0f,
      0.0f,
      0.0f,
      2.0f / -io.DisplaySize.y,
      0.0f,
      0.0f,
      0.0f,
      0.0f,
      -1.0f,
      0.0f,
      -1.0f,
      1.0f,
      0.0f,
      1.0f,
  };
  prog_.use();
  glUniformMatrix4fv(0, 1, GL_FALSE, ortho_projection);
  rs_.apply();
  cbs_.apply();
  dss_.apply();

  va_.bind();
  font_tex_.active(0, GL_TEXTURE_2D);
  font_ss_.bind(0);
  for (int n = 0; n < draw_data->CmdListsCount; ++n) {
    const ImDrawList* cmd_list = draw_data->CmdLists[n];
    const ImDrawIdx* idx_buffer_offset = 0;
    
    // インデックスデータを書き込む
    ib_.bind(GL_ELEMENT_ARRAY_BUFFER);
    ImDrawIdx* ib_data = reinterpret_cast<ImDrawIdx*>(
        glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0,
                         cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx),
                         GL_MAP_WRITE_BIT));
    memcpy(ib_data, cmd_list->IdxBuffer.Data,
           cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

    // 頂点データを書き込む
    vb_.bind(GL_ARRAY_BUFFER);
    ImDrawVert* vb_data = reinterpret_cast<ImDrawVert*>(glMapBufferRange(
        GL_ARRAY_BUFFER, 0, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert),
        GL_MAP_WRITE_BIT));
    memcpy(vb_data, cmd_list->VtxBuffer.Data,
           cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
    glUnmapBuffer(GL_ARRAY_BUFFER);

    // 描画する
    for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
      const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
      if (pcmd->UserCallback) {
        pcmd->UserCallback(cmd_list, pcmd);
      } else {
        glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w),
                  (int)(pcmd->ClipRect.z - pcmd->ClipRect.x),
                  (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
        glDrawElements(
            GL_TRIANGLES, (GLsizei)pcmd->ElemCount,
            sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
            idx_buffer_offset);
      }
      idx_buffer_offset += pcmd->ElemCount;
    }
  }


  glViewport(0, 0, 1280, 720);
  glDisable(GL_SCISSOR_TEST);
}

void Gui::on_mouse_button(int button, int action, int mods) {
  if (action == GLFW_PRESS && button >= 0 && button < IM_ARRAYSIZE(mouse_pressed_)) {
    mouse_pressed_[button] = true;
  }
}

void Gui::on_scroll(double x, double y) {
  auto& io = ImGui::GetIO();
  io.MouseWheelH = static_cast<float>(x);
  io.MouseWheel = static_cast<float>(y);
}

void Gui::on_key(int key, int scancode, int action, int mods) {
  ImGuiIO& io = ImGui::GetIO();
  if (action == GLFW_PRESS) io.KeysDown[key] = true;
  if (action == GLFW_RELEASE) io.KeysDown[key] = false;

  io.KeyCtrl =
      io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
  io.KeyShift =
      io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
  io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
  io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
}

void Gui::on_char(unsigned int c) {
  if (c > 0 && c < 0x10000) {
    ImGui::GetIO().AddInputCharacter((unsigned short)c);
  }
}
}  // namespace rtdemo::gui
