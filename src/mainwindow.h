#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstddef>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>

#include <dear_sink.h>

class MainWindow {
  private:
    GLFWwindow *m_window;
    GLsizei m_window_width  = 1440;
    GLsizei m_window_height = 1024;
    float bottom_margin     = 512.0f;

    bool m_isvalid = false;

    dear_sink_mt_t m_sink;

  public:
    MainWindow();

    void begin_frame();

    void end_frame() const;

    bool should_close() const { return glfwWindowShouldClose(m_window); }
};

#endif // MAINWINDOW_H_
