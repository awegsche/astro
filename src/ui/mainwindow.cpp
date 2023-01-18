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

MainWindow::MainWindow(fs::path const &root, GLsizei width, GLsizei height)
    : m_window_width(width), m_window_height(height), m_root(root) {

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

    m_screen.init();

    m_isvalid = true;

    if (fs::exists(m_root))
        m_batch.load(m_root);
}

void MainWindow::end_frame() const {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(m_window);
}

void MainWindow::begin_frame() {
    constexpr float STATUS_BAR_HEIGHT = 35.0f;

    glfwPollEvents();
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // ---- Central Panel ---------------------------------------------------------------------------
    ImGui::SetNextWindowPos({left_margin, 0.0f});
    ImGui::SetNextWindowSize({
        static_cast<float>(m_window_width - left_margin),
        m_window_height - bottom_margin,
    });
    ImGui::Begin("Central Panel", nullptr,
                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoResize);
    m_screen.display(static_cast<float>(m_window_width - left_margin), m_window_height - bottom_margin,
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoResize);
    ImGui::End();

    // ---- Bottom Panel ----------------------------------------------------------------------------
    // Log Window
    m_sink->draw_imgui(0.0f, m_window_height - bottom_margin, static_cast<float>(m_window_width),
                       bottom_margin - STATUS_BAR_HEIGHT);

    ImGui::SetNextWindowPos({0.0f, m_window_height - STATUS_BAR_HEIGHT});
    ImGui::SetNextWindowSize({(float)m_window_width, STATUS_BAR_HEIGHT});
    // Status Bar
    ImGui::Begin("Status Bar", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoDecoration);
    ImGui::Text("Status Bar");

    // ---- Left Panel ------------------------------------------------------------------------------
    ImGui::SetNextWindowPos({0.0f, 0.0f});
    ImGui::SetNextWindowSize({left_margin, m_window_height - bottom_margin});
    ImGui::Begin("Left Panel", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
    m_batch.draw_imgui(m_screen);
    ImGui::End();

    ImGui::SameLine();
    ImGui::Text("Hello");
    ImGui::End();

    // ---- Floating Windows ------------------------------------------------------------------------
    // and, finally draw all floating windows
    m_batch.draw_floating();
}

auto Config::load_default() -> Config {

#ifdef WIN32
    const char *home = std::getenv("USERPROFILE");
#else
    const char *home = std::getenv("HOME");
#endif

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
        spdlog::error("no config file found at {}", config_path.string());
        return;
    }

    auto toml_conf = toml::parse(config_path.c_str());

    if (toml_conf.contains("font")) {
        auto const &font = toml::find(toml_conf, "font");
        if (font.contains("path")) {
            auto fpath = toml::find<std::string>(font, "path");
            if (fs::exists(fpath))
                font_path = fpath;
            else
                spdlog::warn("Font at {} can't be loaded.", fpath.c_str());
        }
        font_size = toml::find_or(font, "size", DEFAULT_FSIZE);
    }
}

MainWindow::~MainWindow() {
    if (m_isvalid) {
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
    glfwTerminate();
}
