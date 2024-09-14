#version 430 core
#define DEBUG 0

layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_tex_coord;
layout(location = 2) in vec4 a_color;

layout (std140, binding = 0) uniform Global
{
    vec4 g_time;
};

layout (std140, binding = 1) uniform Texture
{
    vec2 texture_size;
    float texture_layer_count;
};

uniform mat4 u_model;
uniform sampler2D u_main_texture;

/* Vertex Data */
in layout(location = 3) vec2 v_tex_coord;
in layout(location = 4) vec4 v_color;

out vec4 color;

void main() {
    vec2 uv = v_tex_coord;
    float d = texture(u_main_texture, v_tex_coord).r;

    float aaf = fwidth(d);
    float alpha = smoothstep(0.5 - aaf, 0.5 + aaf, d);

    color = vec4(v_color.rgb, v_color.a * d);
#if DEBUG == 1
    color = mix(vec4(1, 0, 0, 1), color, color.a);
#endif
}
