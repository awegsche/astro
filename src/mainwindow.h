#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstddef>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <optional>
#include <string>

#include <dear_sink.h>

struct Config {
    std::optional<std::string> font_path = {};
    float font_size                      = 16.0f;

    static auto load_default() -> Config;
};

class MainWindow {
  private:
    // general window parameters
    GLFWwindow *m_window;
    GLsizei m_window_width  = 1440;
    GLsizei m_window_height = 1024;
    float bottom_margin     = 512.0f;

    bool m_isvalid = false;

    dear_sink_mt_t m_sink;

    // config
    Config m_config = {};

  public:
    MainWindow();
    ~MainWindow();

    void begin_frame();

    void end_frame() const;

    bool should_close() const { return glfwWindowShouldClose(m_window); }
};

#endif // MAINWINDOW_H_
