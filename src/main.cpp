#include <cstdlib>
#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <rtdemo/gui.hpp>
#include <rtdemo/logging.hpp>
#include <rtdemo/scene/static_scene.hpp>
#include <rtdemo/tech/forward_shading.hpp>

using namespace rtdemo;

namespace {
void glfw_error_callback(int, const char* desc) {
    RT_LOG_DEBUG("GLFW Error: {}", desc);
}
} // namespace

int main() {
    // init GLFW
    glfwSetErrorCallback((GLFWerrorfun)glfw_error_callback);
    if (!glfwInit()) {
        RT_LOG_DEBUG("failed to glfwInit");
        return EXIT_FAILURE;
    }

    // create GLFW window
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifndef NDEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Rendering Techniques Demo", nullptr, nullptr);
    if (!window) {
        RT_LOG_DEBUG("failed to glfwCreateWindow");
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

#ifdef WIN32
	freopen("CONOUT$", "w", stdout);
#endif

	// init GLEW
    if (glewInit() != GLEW_OK) {
        RT_LOG_DEBUG("failed to glewInit");
        return EXIT_FAILURE;
    }

    RT_LOG_DEBUG("start main");
    {
        // init GUI
        gui::init(window);

        // init scene
        scene::StaticScene scene;
        scene.init();

        // init tech
        tech::ForwardShading shading;
        shading.init();

        // init camera
        const glm::mat4 proj = glm::perspectiveFov(glm::radians(45.f), 1280.f, 720.f, 0.01f, 100.f);
        glm::vec3 camera_pos(0.f, 0.f, 5.f);
        float camera_yaw = 0.f;
        float camera_pitch = 0.f;
        garie::Buffer camera_ubo;
        camera_ubo.create();
        camera_ubo.bind(GL_UNIFORM_BUFFER);
        glBufferStorage(GL_UNIFORM_BUFFER, sizeof(glm::mat4), nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
        void* camera_ubo_ptr = glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            gui::new_frame();

            ImGui::DragFloat("distance", &camera_pos.z, 0.01f, 0.01f, 10.f);
            ImGui::SliderAngle("yaw", &camera_yaw, -180.f, 180.f);
            ImGui::SliderAngle("pitch", &camera_pitch, -90.f, 90.f);

            glViewport(0, 0, 1280, 720);
            glDisable(GL_SCISSOR_TEST);
            glClearColor(0.2f, 0.2f, 0.2f, 1.f);
            glClearDepthf(1.f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            {
                const glm::mat3 rot = glm::yawPitchRoll(camera_yaw, camera_pitch, 0.f);
                const glm::vec3 eye = rot * camera_pos;
                const glm::vec3 front = rot * glm::vec3(0.f, 0.f, -1.f);
                const glm::vec3 up = rot * glm::vec3(0.f, 1.f, 0.f);
                const glm::mat4 view = glm::lookAt(eye, glm::vec3(), up);
                const glm::mat4 view_proj = proj * view;
                memcpy(camera_ubo_ptr, glm::value_ptr(view_proj), sizeof(glm::mat4));
            }

            glEnable(GL_DEPTH_TEST);

            shading.apply();
            scene.apply();
            camera_ubo.bind_base(GL_UNIFORM_BUFFER, 0);
            scene.draw();

            ImGui::Render();

            glfwSwapBuffers(window);
        }
    }
    RT_LOG_DEBUG("finish main");

    gui::terminate();
    glfwTerminate();

#ifdef WIN32
    system("PAUSE");
#else
    // system("read -p \"Press Enter key to continue...\"");
#endif
    return EXIT_SUCCESS;
}
