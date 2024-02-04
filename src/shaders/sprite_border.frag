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
in vec4 v_bounds;
in vec2 v_screen_coord;
in float v_tex_layer;
in vec2 v_protection;
in vec2 v_texel;
in vec4 v_edges;

out vec4 color;

vec2 calculate_uv(vec2 uv, vec2 tex_size) 
{
    vec2 pixels = uv * tex_size;
    vec2 alpha = 0.7 * fwidth(pixels);
    vec2 pixels_fract = fract(pixels);
    vec2 pixels_diff = clamp( .5 / alpha * pixels_fract, 0.0, .5 ) + clamp( .5 / alpha * (pixels_fract - 1) + .5, 0.0, .5 );
    pixels = floor(pixels) + pixels_diff;
    
    return pixels / tex_size;
}

void main() {
    vec2 uv;
    vec2 screen_uv = v_screen_coord;
    
    if(screen_uv.x > v_edges.z)
        uv.x = v_bounds.z - v_protection.x + (screen_uv.x - v_edges.z) * v_texel.x;
    else if(screen_uv.x < v_edges.x)
        uv.x = v_bounds.x + screen_uv.x * v_texel.x;
    else
        uv.x = v_bounds.z - v_protection.x;

    if(screen_uv.y < v_edges.y)
        uv.y = v_bounds.y - screen_uv.y * v_texel.y;
    else if(screen_uv.y > v_edges.w)
        uv.y = v_bounds.w + v_protection.y + -(screen_uv.y - v_edges.w) * v_texel.y;
    else 
        uv.y = v_bounds.w + v_protection.y;

    uv.y = 1 - uv.y;
    vec4 texture_color = texture(u_main_texture, vec3(uv, v_tex_layer));
    color = texture_color;
}
