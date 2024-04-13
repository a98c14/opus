#version 430 core

layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_tex_coord;

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
out vec2 v_tex_coord;

void main() 
{
    mat4 model = projection * view * u_model;
    v_tex_coord = a_tex_coord;
    gl_Position = model * vec4(a_pos, 0, 1);
}