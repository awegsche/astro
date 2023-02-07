#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstddef>
#include <filesystem>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <optional>
#include <string>

#include <dear_sink.h>

#include "photobatch.h"
#include "screen.h"

constexpr GLsizei DEFAULT_WIDTH  = 1440;
constexpr GLsizei DEFAULT_HEIGHT = 1024;

/// The window's config
struct Config {
    std::optional<std::string> font_path = {};
    float font_size                      = 16.0f;

    Config() {}
    explicit Config(std::filesystem::path const &config_file);

    static auto load_default() -> Config;
};

class MainWindow {
  private:
    // general window parameters
    GLFWwindow *m_window;
    GLsizei m_window_width;
    GLsizei m_window_height;
    float bottom_margin = 256.0f;
    float left_margin   = 256.0f;

    bool m_isvalid = false;

    dear_sink_mt_t m_sink = nullptr;

    // config
    Config m_config = {};

    // data
    std::filesystem::path m_root = {};
    //BatchUi m_batch              = {};

  public:
    // public for debugging
    Screen m_screen = {};

    explicit MainWindow(std::filesystem::path const &root, GLsizei width = DEFAULT_WIDTH, GLsizei height = DEFAULT_HEIGHT);

    ~MainWindow();

    void begin_frame();

    void end_frame() const;

    [[nodiscard]]
    bool should_close() const { return glfwWindowShouldClose(m_window); }
};

#endif // MAINWINDOW_H_
