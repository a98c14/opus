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
    float u_alpha;
    vec4 u_color;
};

layout(std140, binding = 5) buffer SpriteBoundBuffer
{
    vec4 sprite_bounds[];
};

uniform mat4 u_model;
uniform sampler2DArray u_main_texture;

/* Vertex Data */
out vec4 v_bounds;
out vec2 v_tex_coord;
out float v_tex_layer;

void main() 
{
    v_bounds = sprite_bounds[u_sprite_index];
    gl_Position = projection * view * u_model * vec4(a_pos, 1.0);

    float x = ((a_tex_coord.x * v_bounds.z) + v_bounds.x) / texture_size.x;
    float y = 1 - (((1 - a_tex_coord.y) * v_bounds.w) + v_bounds.y) / texture_size.y;
    v_tex_coord = vec2(x, y);
    v_tex_layer = 0;
    v_tex_layer = max(0.0, min(texture_layer_count - 1.0, floor(u_texture_layer_index + 0.5)));
}