#include "mainwindow.h"
#include <filesystem>
#include <spdlog/spdlog.h>

MainWindow::MainWindow() {
    using std::cerr, std::endl;

    if (!glfwInit())
        return;

    // create window
    m_window = glfwCreateWindow(m_window_width, m_window_height, "Asto Viewer", NULL, NULL);

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

    // TODO: this is not portable, move to config file or whatever
    static const char fira_path[] = "C:\\Users\\andiw\\AppData\\Local\\Microsoft\\Windows\\Fonts\\FiraCode-SemiBold.ttf";

    if (std::filesystem::exists(fira_path)) {
        io.Fonts->AddFontFromFileTTF(fira_path, 20.0f);
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
