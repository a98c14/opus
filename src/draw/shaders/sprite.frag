#version 430 core

layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_tex_coord;
layout(location = 2) in vec3 a_color;

struct ShaderData
{
    mat4 model;
    vec4 bounds;
    vec4 color;
};

layout (std140, binding = 0) uniform Global
{
    mat4 g_projection;
    mat4 g_view;
    vec4 g_time;
};

layout (std140, binding = 1) uniform Texture
{
    vec2 texture_size;
    float texture_layer_count;
};

layout (std140, binding = 2) buffer Custom
{
    ShaderData data[];
};

uniform sampler2D u_main_texture;

/* Vertex Data */
in vec2 v_tex_coord;
out vec4 color;

void main() {
    vec2 uv = v_tex_coord;
    vec4 texture_color = texture(u_main_texture, uv);
    color = vec4(texture_color.xyz, 1.0);
    // color = vec4(uv.x, uv.y, 1.0, 1.0);
}
