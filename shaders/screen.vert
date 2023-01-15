#version 330

layout(location = 0) in vec2 attr_position;
layout(location = 1) in vec2 attr_texcoord;

out vec2 var_tex_coord;

void main()
{
  gl_Position = vec4(attr_position, 0.0, 1.0);
  var_tex_coord = attr_texcoord;
}
