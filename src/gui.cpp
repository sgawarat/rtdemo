#include <rendering_techniques/stable.hpp>

namespace gui {
namespace {
GLFWwindow* window_ = nullptr;
double time_ = 0.0;
bool mouse_pressed_[3] = {};
float mouse_wheel_ = 0.f;

static const char* get_clipboard_text(void* user_data) {
    return glfwGetClipboardString((GLFWwindow*)user_data);
}

void set_clipboard_text(void* user_data, const char* text) {
    glfwSetClipboardString((GLFWwindow*)user_data, text);
}

void mouse_button_callback(GLFWwindow*, int button, int action, int /*mods*/) {
    if (action == GLFW_PRESS && button >= 0 && button < 3) {
        mouse_pressed_[button] = true;
    }
}

void scroll_callback(GLFWwindow*, double /*xoffset*/, double yoffset) {
    mouse_wheel_ += (float)yoffset;
}

void key_callback(GLFWwindow*, int key, int, int action, int /*rmods*/) {
    ImGuiIO& io = ImGui::GetIO();
    if (action == GLFW_PRESS) {
        io.KeysDown[key] = true;
    }
    if (action == GLFW_RELEASE) {
        io.KeysDown[key] = false;
    }

    io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
    io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
    io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
    // io.KeyKeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
}

void char_callback(GLFWwindow*, unsigned int c) {
    ImGuiIO& io = ImGui::GetIO();
    if (c > 0 && c < 0x10000) {
        io.AddInputCharacter((unsigned short)c);
    }
}

void render_drawlists(ImDrawData* draw_data) {
    // ImGuiIO& io = ImGui::GetIO();
    // int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
    // int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
    // if (fb_width == 0 || fb_height == 0) return;
    // draw_data->ScaleClipRects(io.DisplayFramebufferScale);


    // glEnable(GL_BLEND);
    // glBlendEquation(GL_FUNC_ADD);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // glDisable(GL_CULL_FACE);
    // glDisable(GL_DEPTH_TEST);
    // glEnable(GL_SCISSOR_TEST);

    // glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
    // const float ortho_projection[4][4] = {
    //     { 2.0f/io.DisplaySize.x, 0.0f,                   0.0f, 0.0f },
    //     { 0.0f,                  2.0f/-io.DisplaySize.y, 0.0f, 0.0f },
    //     { 0.0f,                  0.0f,                  -1.0f, 0.0f },
    //     {-1.0f,                  1.0f,                   0.0f, 1.0f },
    // };
    // glUseProgram(g_ShaderHandle);
    // glUniform1i(g_AttribLocationTex, 0);
    // glUniformMatrix4fv(g_AttribLocationProjMtx, 1, GL_FALSE, &ortho_projection[0][0]);
    // glBindVertexArray(g_VaoHandle);

    // for (int n = 0; n < draw_data->CmdListsCount; n++)
    // {
    //     const ImDrawList* cmd_list = draw_data->CmdLists[n];
    //     const ImDrawIdx* idx_buffer_offset = 0;

    //     glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle);
    //     glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert), (const GLvoid*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW);

    //     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ElementsHandle);
    //     glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx), (const GLvoid*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW);

    //     for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
    //     {
    //         const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
    //         if (pcmd->UserCallback)
    //         {
    //             pcmd->UserCallback(cmd_list, pcmd);
    //         }
    //         else
    //         {
    //             glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
    //             glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
    //             glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer_offset);
    //         }
    //         idx_buffer_offset += pcmd->ElemCount;
    //     }
    // }

    // // Restore modified GL state
    // glUseProgram(last_program);
    // glBindTexture(GL_TEXTURE_2D, last_texture);
    // glActiveTexture(last_active_texture);
    // glBindVertexArray(last_vertex_array);
    // glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, last_element_array_buffer);
    // glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
    // glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb, last_blend_src_alpha, last_blend_dst_alpha);
    // if (last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
    // if (last_enable_cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
    // if (last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
    // if (last_enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
    // glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
    // glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
}
} // namespace

void init(GLFWwindow* window) {
    ImGuiIO& io = ImGui::GetIO();
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

    io.RenderDrawListsFn = render_drawlists;
    io.SetClipboardTextFn = set_clipboard_text;
    io.GetClipboardTextFn = get_clipboard_text;
    io.ClipboardUserData = window;
#ifdef _WIN32
    io.ImeWindowHandle = glfwGetWin32Window(window);
#endif

    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCharCallback(window, char_callback);

    window_ = window;
}

void terminate() {
    ImGui::Shutdown();
}

void new_frame() {
    ImGuiIO& io = ImGui::GetIO();

    // Setup display size (every frame to accommodate for window resizing)
    int w, h;
    int display_w, display_h;
    glfwGetWindowSize(window_, &w, &h);
    glfwGetFramebufferSize(window_, &display_w, &display_h);
    io.DisplaySize = ImVec2((float)w, (float)h);
    io.DisplayFramebufferScale = ImVec2(w > 0 ? ((float)display_w / w) : 0, h > 0 ? ((float)display_h / h) : 0);

    // Setup time step
    double current_time =  glfwGetTime();
    io.DeltaTime = time_ > 0.0 ? (float)(current_time - time_) : (float)(1.0f/60.0f);
    time_ = current_time;

    // Setup inputs
    // (we already got mouse wheel, keyboard keys & characters from glfw callbacks polled in glfwPollEvents())
    if (glfwGetWindowAttrib(window_, GLFW_FOCUSED)) {
        double mouse_x, mouse_y;
        glfwGetCursorPos(window_, &mouse_x, &mouse_y);
        io.MousePos = ImVec2((float)mouse_x, (float)mouse_y);   // Mouse position in screen coordinates (set to -1,-1 if no mouse / on another screen, etc.)
    } else {
        io.MousePos = ImVec2(-1, -1);
    }

    for (int i = 0; i < 3; i++) {
        io.MouseDown[i] = mouse_pressed_[i] || glfwGetMouseButton(window_, i) != 0;    // If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
        mouse_pressed_[i] = false;
    }

    io.MouseWheel = mouse_wheel_;
    mouse_wheel_ = 0.0f;

    // Hide OS mouse cursor if ImGui is drawing it
    glfwSetInputMode(window_, GLFW_CURSOR, io.MouseDrawCursor ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);

    // Start the frame
    ImGui::NewFrame();
}
}  // namespace gui
