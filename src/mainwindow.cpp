#include "mainwindow.h"
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

    // TODO: this is not portable, move to config tile or whatever
    auto &io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("C:\\Users\\andiw\\AppData\\Local\\Microsoft\\Fonts\\FiraCode-SemiBold.ttf", 30.0f);

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

    m_isvalid = true;
}
