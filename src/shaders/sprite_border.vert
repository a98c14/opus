#version 430 core

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_color;
layout(location = 2) in vec2 a_tex_coord;

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
    int u_sprite_index;
    int u_texture_layer_index;
    vec2 u_size;
    vec4 u_color;
    int u_protection;
};

layout(std140, binding = 5) buffer SpriteBoundBuffer
{
    vec4 sprite_bounds[];
};

uniform mat4 u_model;
uniform sampler2DArray u_main_texture;

/* Vertex Data */
out vec4 v_bounds;
out vec2 v_screen_coord;
out vec2 v_protection;
out float v_tex_layer;
out vec2 v_texel;
out vec4 v_edges;

void main() 
{
    vec4 bounds = sprite_bounds[u_sprite_index];
    gl_Position = projection * view * u_model * vec4(a_pos, 1.0);

    v_protection = vec2(u_protection.x / texture_size.x, u_protection / texture_size.y); 
    v_bounds = vec4(bounds.x / texture_size.x, (bounds.y + bounds.w) / texture_size.y, (bounds.x + bounds.z) / texture_size.x, bounds.y / texture_size.y);
    v_screen_coord = a_tex_coord;
    v_tex_layer = 0;
    v_tex_layer = max(0.0, min(texture_layer_count - 1.0, floor(u_texture_layer_index + 0.5)));
    v_texel = vec2(u_size.x / texture_size.x, u_size.y / texture_size.y);

    float dx = 1. / u_size.x;
    float dy = 1. / u_size.y;

    v_edges.x = u_protection * dx;
    v_edges.z = 1 - u_protection * dx;
    v_edges.y = u_protection * dy;
    v_edges.w = 1 - u_protection * dy;
}