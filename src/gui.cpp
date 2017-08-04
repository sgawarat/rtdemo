#include <rtdemo/gui.hpp>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>

namespace rtdemo {
namespace gui {
namespace {
GLFWwindow* window_ = nullptr;
double time_ = 0.0;
bool mouse_pressed_[3] = {};
float mouse_wheel_ = 0.f;
GLuint prog_ = 0;
GLuint vbo_ = 0;
GLuint ibo_ = 0;
GLuint vao_ = 0;
GLuint font_tex_ = 0;
GLint tex_loc_ = -1;
GLint proj_loc_ = -1;

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

bool create_gl_resources() {
    const GLchar *vertex_shader =
        "#version 330\n"
        "uniform mat4 ProjMtx;\n"
        "in vec2 Position;\n"
        "in vec2 UV;\n"
        "in vec4 Color;\n"
        "out vec2 Frag_UV;\n"
        "out vec4 Frag_Color;\n"
        "void main()\n"
        "{\n"
        "	Frag_UV = UV;\n"
        "	Frag_Color = Color;\n"
        "	gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
        "}\n";

    const GLchar* fragment_shader =
        "#version 330\n"
        "uniform sampler2D Texture;\n"
        "in vec2 Frag_UV;\n"
        "in vec4 Frag_Color;\n"
        "out vec4 Out_Color;\n"
        "void main()\n"
        "{\n"
        "	Out_Color = Frag_Color * vec4(1, 1, 1, texture(Texture, Frag_UV.st));\n"
        "}\n";

    GLuint vert = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert, 1, &vertex_shader, 0);
    glCompileShader(vert);
    GLint vert_result = 0;
    glGetShaderiv(vert, GL_COMPILE_STATUS, &vert_result);
    if (vert_result == GL_FALSE) {
        GLsizei length = 0;
        GLchar log[1024];
        glGetShaderInfoLog(vert, 1024, &length, log);
        std::cout << "failed to compile vertex shader: " << log << "\n";
        glDeleteShader(vert);
        return false;
    }

    GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag, 1, &fragment_shader, 0);
    glCompileShader(frag);
    GLint frag_result = 0;
    glGetShaderiv(frag, GL_COMPILE_STATUS, &frag_result);
    if (frag_result == GL_FALSE) {
        GLsizei length = 0;
        GLchar log[1024];
        glGetShaderInfoLog(frag, 1024, &length, log);
        std::cout << "failed to compile fragment shader: " << log << "\n";
        glDeleteShader(vert);
        glDeleteShader(frag);
        return false;
    }

    prog_ = glCreateProgram();
    glAttachShader(prog_, vert);
    glAttachShader(prog_, frag);
    glLinkProgram(prog_);
    glDeleteShader(vert);
    glDeleteShader(frag);
    GLint prog_result = 0;
    glGetProgramiv(prog_, GL_LINK_STATUS, &prog_result);
    if (prog_result == GL_FALSE) {
        GLsizei length = 0;
        GLchar log[1024];
        glGetProgramInfoLog(frag, 1024, &length, log);
        std::cout << "failed to link shader program: " << log << "\n";
        glDeleteProgram(prog_);
        prog_ = 0;
        return false;
    }

    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ibo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
    glBufferData(GL_ARRAY_BUFFER, 4 * 1024 * sizeof(ImDrawVert), nullptr, GL_DYNAMIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * 1024 * sizeof(ImDrawIdx), nullptr, GL_DYNAMIC_DRAW);

    const GLint pos_loc = glGetAttribLocation(prog_, "Position");
    const GLint uv_loc = glGetAttribLocation(prog_, "UV");
    const GLint color_loc = glGetAttribLocation(prog_, "Color");
    glEnableVertexAttribArray(pos_loc);
    glEnableVertexAttribArray(uv_loc);
    glEnableVertexAttribArray(color_loc);
    glVertexAttribPointer(pos_loc, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)offsetof(ImDrawVert, pos));
    glVertexAttribPointer(uv_loc, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)offsetof(ImDrawVert, uv));
    glVertexAttribPointer(color_loc, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid*)offsetof(ImDrawVert, col));

    tex_loc_ = glGetUniformLocation(prog_, "Texture");
    proj_loc_ = glGetUniformLocation(prog_, "ProjMtx");

    // Build texture atlas
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height;
    io.Fonts->AddFontFromFileTTF("data/fonts/migu-1m-regular.ttf", 14.f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    io.Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);

    // Upload texture to graphics system
    glGenTextures(1, &font_tex_);
    glBindTexture(GL_TEXTURE_2D, font_tex_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R8, width, height);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED, GL_UNSIGNED_BYTE, pixels);

    // Store our identifier
    io.Fonts->TexID = (void *)(intptr_t)font_tex_;

    return true;
}

void render_drawlists(ImDrawData* draw_data) {
    ImGuiIO& io = ImGui::GetIO();
    int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
    int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
    if (fb_width == 0 || fb_height == 0) return;
    draw_data->ScaleClipRects(io.DisplayFramebufferScale);

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);

    glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
    const float ortho_projection[4][4] = {
        { 2.0f/io.DisplaySize.x, 0.0f,                   0.0f, 0.0f },
        { 0.0f,                  2.0f/-io.DisplaySize.y, 0.0f, 0.0f },
        { 0.0f,                  0.0f,                  -1.0f, 0.0f },
        {-1.0f,                  1.0f,                   0.0f, 1.0f },
    };
    glUseProgram(prog_);
    glUniform1i(tex_loc_, 0);
    glUniformMatrix4fv(proj_loc_, 1, GL_FALSE, &ortho_projection[0][0]);
    glBindVertexArray(vao_);

    for (int n = 0; n < draw_data->CmdListsCount; n++) {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        const ImDrawIdx* idx_buffer_offset = 0;

        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        // GLvoid* vb_data = glMapBufferRange(GL_ARRAY_BUFFER, 0, (GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
        GLvoid* vb_data = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        GLenum err = glGetError();
        memcpy(vb_data, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
        glUnmapBuffer(GL_ARRAY_BUFFER);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
        // GLvoid* ib_data = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, (GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
        GLvoid* ib_data = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
        memcpy(ib_data, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback) {
                pcmd->UserCallback(cmd_list, pcmd);
            } else {
                glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
                glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
                glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer_offset);
            }
            idx_buffer_offset += pcmd->ElemCount;
        }
    }
}
} // namespace

bool init(GLFWwindow* window) {
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

    if (!create_gl_resources()) return false;

    window_ = window;
    return true;
}

void terminate() {
    if (vao_) {
        glDeleteVertexArrays(1, &vao_);
        vao_ = 0;
    }
    if (vbo_) {
        glDeleteBuffers(1, &vao_);
        vbo_ = 0;
    }
    if (ibo_) {
        glDeleteBuffers(1, &ibo_);
        ibo_ = 0;
    }
    if (prog_) {
        glDeleteProgram(prog_);
        prog_ = 0;
    }
    if (font_tex_) {
        glDeleteTextures(1, &font_tex_);
        ImGui::GetIO().Fonts->TexID = 0;
        font_tex_ = 0;
    }
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
}  // namespace rtdemo
