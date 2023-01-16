#ifndef SCREEN_H_
#define SCREEN_H_

#include <GL/glew.h>
#include <imgui.h>

/// Screen contains the image to be shown to the user.
///
/// This can be a currently open raw file for examination,
/// an already processed image in the process
/// or the final output image for tonemapping.
class Screen {
    int m_width  = 1024;
    int m_height = 1024;

    float m_gamma      = 1.0f / 2.2f;
    float m_whitepoint = 0.3f;
    float m_blackpoint = 0.0f;

    GLuint m_glslVS              = 0;
    GLuint m_glslFS              = 0;
    GLuint m_glslProgram         = 0;
    GLuint m_position_location   = 0;
    GLuint m_texcoord_location   = 0;
    GLuint m_gamma_location      = 0;
    GLuint m_whitepoint_location = 0;
    GLuint m_blackpoint_location = 0;

    GLuint m_pbo            = 0;
    GLuint m_texture        = 0;
    GLuint m_vbo_attributes = 0;
    GLuint m_vbo_indices    = 0;
    GLuint m_framebuffer    = 0;
    GLuint m_texturebuffer  = 0;
    GLuint rbo              = 0;

    bool m_valid = false;

    /// setup opengl shader program for display
    void init_GLSL();

  public:
    Screen();

    void init();

    void load_data_cpu(int width, int height, const float *data);

    void display(float w, float h, ImGuiWindowFlags flags = 0) const;

    void render() const;
};

#endif // SCREEN_H_
