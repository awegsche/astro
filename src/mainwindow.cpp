#include "mainwindow.h"
#include "toml/get.hpp"
#include "toml/parser.hpp"
#include <cstdlib>
#include <filesystem>
#include <imgui.h>
#include <optional>
#include <spdlog/spdlog.h>
#include <stdlib.h>

#include <toml.hpp>

using std::cerr, std::endl, std::cout;
namespace fs = std::filesystem;

constexpr char CONFIG_PATH[]  = ".config/astro/config.toml";
constexpr float DEFAULT_FSIZE = 16.0f;

MainWindow::MainWindow() {

    // try to get config
    m_config = Config::load_default();

    if (!glfwInit())
        return;

    // create window
    m_window = glfwCreateWindow(m_window_width, m_window_height, "Astro Viewer", NULL, NULL);

    if (!m_window) {
        cerr << "Couldn create window" << endl;
        return;
    }

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);

    // init GLEW
    if (glewInit() != GL_NO_ERROR) {
        cerr << "Couldn't initialise GLEW" << endl;
        return;
    }

    // setup ImGui
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init();

    auto &io = ImGui::GetIO();

    if (m_config.font_path.has_value()) {
        auto path = *m_config.font_path;
        if (std::filesystem::exists(path)) {
            io.Fonts->AddFontFromFileTTF(path.c_str(), m_config.font_size);
        }
    } else {
        ImFontConfig c;
        c.SizePixels = m_config.font_size;
        io.Fonts->AddFontDefault(&c);
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // init OpenGL
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glViewport(0, 0, m_window_width, m_window_height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    m_sink = dear_sink_mt();

    spdlog::info("hello to sink");
    spdlog::warn("warn from main window");

    m_isvalid = true;
}

void MainWindow::end_frame() const {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(m_window);
}

void MainWindow::begin_frame() {
    glfwPollEvents();
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // docking
    m_sink->draw_imgui(0.0f, m_window_height - bottom_margin, static_cast<float>(m_window_width), bottom_margin);
}

auto Config::load_default() -> Config {

    const char *home = std::getenv("HOME");

    if (!home) {
        spdlog::error("error loading default config file, $HOME couldn't be found");
        return {};
    }

    fs::path config_path{home};
    config_path /= CONFIG_PATH;

    return Config{config_path};
}

Config::Config(std::filesystem::path const &config_path) {
    if (!fs::exists(config_path)) {
        spdlog::error("no config file found at {}", config_path);
        return;
    }

    auto toml_conf = toml::parse(config_path.c_str());

    Config conf{};
    if (toml_conf.contains("font")) {
        auto const &font = toml::find(toml_conf, "font");
        if (font.contains("path")) {
            auto fpath = toml::find<std::string>(font, "path");
            if (fs::exists(fpath))
                conf.font_path = fpath;
        }
        conf.font_size = toml::find_or(font, "size", DEFAULT_FSIZE);
    }
}

MainWindow::~MainWindow() {
    if (m_isvalid) {
        cout << ("shutting down window") << endl;
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
    cout << ("shutting down glfw") << endl;
    glfwTerminate();
}
