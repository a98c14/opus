#version 430 core

layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_tex_coord;
layout(location = 2) in flat int a_instance_id;

layout (std140, binding = 0) uniform Global
{
    vec4 g_time;
};

layout (std140, binding = 1) uniform Texture
{
    vec2 texture_size;
    float texture_layer_count;
};

layout (std140, binding = 2) uniform Camera
{
    mat4 projection;
    mat4 view;
};

layout (std140, binding = 4) uniform Custom
{
    vec4 u_color;
};

uniform mat4 u_model;
uniform sampler2D u_main_texture;

/* Vertex Data */
in vec2 v_tex_coord;
out vec4 color;

void main() {
    vec2 uv = v_tex_coord;
    vec4 texture_color = texture(u_main_texture, uv);
    color = vec4(texture_color);
    // color = vec4(v_tex_coord.x, v_tex_coord.y, 1, 1);
}
