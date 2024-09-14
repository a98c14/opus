#version 430 core

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

/* Vertex Data */
in layout(location = 3) vec2 v_tex_coord;
in layout(location = 4) vec4 v_color;

out vec4 color;

void main() {
    float softness = 1;
    vec2 uv = v_tex_coord;
    uv = uv * 2.0 - 1.0;
    uv = abs(uv);
    vec2 p = fwidth(uv);
    float a_x = 1 - smoothstep(1.0 - softness * p.x, 1.0, uv.x);
    float a_y = 1 - smoothstep(1.0 - softness * p.y, 1.0, uv.y);
    float a = min(a_x, a_y) * max(a_x, a_y);
    color = vec4(v_color.xyz, a * v_color.a);
}
