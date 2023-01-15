#include "screen.h"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <imgui.h>
#include <spdlog/spdlog.h>
#include <sstream>
#include <string>

GLuint load_shader(const std::filesystem::path &path, GLuint shader_type) {
    std::ifstream vert_file(path);

    if (!vert_file.is_open()) {
        spdlog::error("can't load shader file '{}'", path.string().c_str());
        return 0;
    }

    std::stringstream ss;
    ss << vert_file.rdbuf();
    auto src = ss.str();

    GLuint id = glCreateShader(shader_type);

    if (id) {
        auto len = static_cast<GLsizei>(src.size());

        const GLchar *fs = src.c_str();
        glShaderSource(id, 1, &fs, &len);
        glCompileShader(id);
    }
    return id;
}

void Screen::init_GLSL() {
    GLint vs_compiled = 0;
    GLint fs_compiled = 0;

    m_glslVS = load_shader("screen.vert", GL_VERTEX_SHADER);
    glGetShaderiv(m_glslVS, GL_COMPILE_STATUS, &vs_compiled);

    m_glslFS = load_shader("screen.frag", GL_FRAGMENT_SHADER);
    glGetShaderiv(m_glslFS, GL_COMPILE_STATUS, &fs_compiled);

    m_glslProgram = glCreateProgram();
    if (m_glslProgram) {
        GLint program_linked = 0;

        if (m_glslVS && vs_compiled) {
            glAttachShader(m_glslProgram, m_glslVS);
        }

        if (m_glslFS && fs_compiled) {
            glAttachShader(m_glslProgram, m_glslFS);
        }

        glLinkProgram(m_glslProgram);
        glGetProgramiv(m_glslProgram, GL_LINK_STATUS, &program_linked);

        if (program_linked) {
            glUseProgram(m_glslProgram);

            m_position_location   = glGetAttribLocation(m_glslProgram, "attr_position");
            m_texcoord_location   = glGetAttribLocation(m_glslProgram, "attr_texcoord");
            m_gamma_location      = glGetUniformLocation(m_glslProgram, "gamma");
            m_whitepoint_location = glGetUniformLocation(m_glslProgram, "white_point");
            m_blackpoint_location = glGetUniformLocation(m_glslProgram, "black_point");

            glUniform1i(glGetUniformLocation(m_glslProgram, "sampler_hdr"), 0);
            glUniform1f(m_gamma_location, m_gamma);
            glUniform1f(m_whitepoint_location, m_whitepoint);
            glUniform1f(m_blackpoint_location, m_blackpoint);
            glUseProgram(0);
        }
    }
}

Screen::Screen() {}

void Screen::init() {
    glGenBuffers(1, &m_pbo);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pbo);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, m_width * m_height * sizeof(float) * 4, (void *)0, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    glGenTextures(1, &m_texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    init_GLSL();

    // clang-format off
        const float vertex_pos_uv[16] = {
            -1.0f, -1.0f,  0.0f, 0.0f,
             1.0f, -1.0f,  1.0f, 0.0f,
             1.0f,  1.0f,  1.0f, 1.0f,
            -1.0f,  1.0f,  0.0f, 1.0f,
        };
        const unsigned int indices[6] = {
            0, 1, 2,
            2, 3, 0
        };
    // clang-format on
    //
    glGenBuffers(1, &m_vbo_attributes);
    glGenBuffers(1, &m_vbo_indices);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_attributes);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)sizeof(float) * 16, (GLvoid const *)vertex_pos_uv, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo_indices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)sizeof(float) * 6, (GLvoid const *)indices, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(m_position_location, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (GLvoid *)0);
    glVertexAttribPointer(m_texcoord_location, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (GLvoid *)(sizeof(float) * 2));

    glGenFramebuffers(1, &m_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);

    glGenTextures(1, &m_texturebuffer);
    glBindTexture(GL_TEXTURE_2D, m_texturebuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texturebuffer, 0);
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    m_valid = true;
}

void Screen::load_data_cpu(int width, int height, const void *data) {
    assert(m_valid);

    m_width  = width;
    m_height = height;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, data);
    // glTextureStorage2D(m_texture, 1, GL_RGBA32F, width, height);
    // glTextureSubImage2D(m_texture, 0, 0, 0, width, height, GL_RGBA, GL_FLOAT, data);
    glBindTexture(GL_TEXTURE_2D, 0);
    //glGenTextures(1, &m_texturebuffer);

    glBindTexture(GL_TEXTURE_2D, m_texturebuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void draw_callback(const ImDrawList *parent_list, const ImDrawCmd *cmd) { glViewport(0, 0, 400, 400); }

void Screen::render() const{
    
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
    glClearColor(0.1f, 0.9f, 0.1f, 1.0f);
    glViewport(0, 0, m_width, m_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // we're not using the stencil buffer now

    glEnable(GL_DEPTH_TEST);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_attributes);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo_indices);

    glEnableVertexAttribArray(m_position_location);
    glEnableVertexAttribArray(m_texcoord_location);

    glUseProgram(m_glslProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture);

    glDrawElements(GL_TRIANGLES, (GLsizei)6, GL_UNSIGNED_INT, (const GLvoid *)0);

    glUseProgram(0);

    glDisableVertexAttribArray(m_position_location);
    glDisableVertexAttribArray(m_texcoord_location);

    // second pass
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
}

void Screen::display(float x, float y, float w, float h) const {
    assert(m_valid);

    // fit image
    float scale   = std::min(1.0f, std::min(w / static_cast<float>(m_width), h / static_cast<float>(m_height)));
    float image_w = m_width * scale;
    float image_h = m_height * scale;

    render();


    ImGui::SetNextWindowPos({x, y});
    ImGui::SetNextWindowSize({w, h});
    ImGui::Begin("Screen", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoCollapse);
    //ImVec2 pos = ImGui::GetCursorPos();
    //ImDrawList *drawList = ImGui::GetWindowDrawList();
    //drawList->AddImage((void *)m_texturebuffer, pos, {pos.x + 512, pos.y + 512}, {1, 0}, {0, 1});
    ImGui::BeginChild("sc0");
    ImGui::Image(reinterpret_cast<ImTextureID>(m_texturebuffer), {image_w, image_h}, {1, 0}, {0, 1});
    ImGui::EndChild();
    ImGui::End();
}
