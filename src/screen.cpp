#include "screen.h"
#include <cassert>
#include <imgui.h>
#include <string>

void Screen::init_GLSL() {
    static const std::string vs_src = "#version 330\n"
                                      "layout(location = 0) in vec2 attr_position;\n"
                                      "layout(location = 1) in vec2 attr_texcoord;\n"
                                      "out vec2 var_tex_coord;\n"
                                      "void main()\n"
                                      "{\n"
                                      "  gl_Position = vec4(attr_position, 0.0, 1.0);\n"
                                      "  var_tex_coord = attr_texcoord;\n"
                                      "}\n";

    static const std::string fs_src = "#version 330\n"
                                      "uniform sampler2D sampler_hdr;\n"
                                      "in vec2 var_tex_coord;\n"
                                      "layout(location = 0, index = 0) out vec4 out_color;\n"
                                      "void main()\n"
                                      "{\n"
                                      "  vec3 hdr_color = texture(sampler_hdr, var_tex_coord).rgb;\n"
                                      "  out_color = vec4(hdr_color, 1.0);\n"
                                      "}\n";
    GLint vs_compiled               = 0;
    GLint fs_compiled               = 0;

    m_glslVS = glCreateShader(GL_VERTEX_SHADER);
    if (m_glslVS) {
        GLsizei len      = (GLsizei)vs_src.size();
        const GLchar *vs = vs_src.c_str();
        glShaderSource(m_glslVS, 1, &vs, &len);
        glCompileShader(m_glslVS);

        glGetShaderiv(m_glslVS, GL_COMPILE_STATUS, &vs_compiled);
    }
    m_glslFS = glCreateShader(GL_FRAGMENT_SHADER);
    if (m_glslFS) {
        GLsizei len      = (GLsizei)fs_src.size();
        const GLchar *fs = fs_src.c_str();
        glShaderSource(m_glslFS, 1, &fs, &len);
        glCompileShader(m_glslFS);

        glGetShaderiv(m_glslFS, GL_COMPILE_STATUS, &fs_compiled);
    }

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

            m_position_location = glGetAttribLocation(m_glslProgram, "attr_position");
            m_texcoord_location = glGetAttribLocation(m_glslProgram, "attr_texcoord");

            glUniform1i(glGetUniformLocation(m_glslProgram, "sampler_hdr"), 0);
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
    //glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    init_GLSL();

    // clang-format off
        const float vertex_pos_uv[16] = {
            -0.5f, -1.0f,  0.0f, 0.0f,
             1.0f, -1.0f,  1.0f, 0.0f,
             1.0f,  1.0f,  1.0f, 1.0f,
            -0.5f,  1.0f,  0.0f, 1.0f,
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
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)sizeof(float) * 16, (GLvoid const *)vertex_pos_uv, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo_indices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)sizeof(float) * 6, (GLvoid const *)indices, GL_STATIC_DRAW);

    glVertexAttribPointer(m_position_location, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (GLvoid *)0);
    glVertexAttribPointer(m_texcoord_location, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (GLvoid *)(sizeof(float) * 2));
    m_valid = true;
}

void Screen::load_data_cpu(int width, int height, const void *data) {
    assert(m_valid);

    m_width = width;
    m_height = height;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture);
     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, data);
    //glTextureStorage2D(m_texture, 1, GL_RGBA32F, width, height);
    //glTextureSubImage2D(m_texture, 0, 0, 0, width, height, GL_RGBA, GL_FLOAT, data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Screen::display(float x, float y, float w, float h) const {
    assert(m_valid);

    ImGui::SetNextWindowPos({x, y});
    ImGui::SetNextWindowSize({w, h});
    ImGui::Begin("Screen", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoCollapse);
    ImGui::BeginChild("sc0");
    ImGui::Image(reinterpret_cast<ImTextureID>(m_texture), {(float)m_width, (float)m_height}, {1, 0}, {0, 1});
    ImGui::EndChild();
    ImGui::End();

    return;
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
}
