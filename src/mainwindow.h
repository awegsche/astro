#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstddef>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>

class MainWindow {
  private:
    GLFWwindow *m_window;
    GLsizei m_window_width  = 1440;
    GLsizei m_window_height = 1024;
    float bottom_margin     = 512.0f;

    bool m_isvalid = false;

  public:
    MainWindow();

    void begin_frame() {
        glfwPollEvents();
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // docking
        ImGui::SetNextWindowPos({0.0f, bottom_margin});
        ImGui::SetNextWindowSize({m_window_width, bottom_margin});
        ImGui::Begin("Log");
        ImGui::End();
    }
    void end_frame() const;
};

#endif // MAINWINDOW_H_
