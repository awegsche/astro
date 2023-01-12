#ifndef SCREEN_H_
#define SCREEN_H_

#include <GL/glew.h>

/// Screen contains the image to be shown to the user.
///
/// This can be a currently open raw file for examination,
/// an already processed image in the process
/// or the final output image for tonemapping.
class Screen {
    int m_width  = 1024;
    int m_height = 1024;

    GLuint m_glslVS            = 0;
    GLuint m_glslFS            = 0;
    GLuint m_glslProgram       = 0;
    GLuint m_position_location = 0;
    GLuint m_texcoord_location = 0;

    GLuint m_pbo            = 0;
    GLuint m_texture        = 0;
    GLuint m_vbo_attributes = 0;
    GLuint m_vbo_indices    = 0;

    bool m_valid = false;

    /// setup opengl shader program for display
    void init_GLSL();

  public:
    Screen();

    void init();

    void load_data_cpu(int width, int height, const void *data);

    void display(float x, float y, float w, float h) const;
};

#endif // SCREEN_H_
