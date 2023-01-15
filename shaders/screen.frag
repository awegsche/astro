#version 330

uniform sampler2D sampler_hdr;
uniform float gamma;
uniform float black_point;
uniform float white_point;

in vec2 var_tex_coord;
layout(location = 0, index = 0) out vec4 out_color;

// comment
void main()
 {
   vec3 hdr_color = texture(sampler_hdr, var_tex_coord).rgb;
   hdr_color = (hdr_color - vec3(black_point))/vec3(white_point-black_point);
   hdr_color = pow(hdr_color, vec3(gamma));
   out_color = vec4(hdr_color, 1.0);
}
