#version 430 core

layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_tex_coord;
layout(location = 2) in vec3 a_color;

struct ShaderData
{
    mat4 model;
    vec4 color;
    vec4 roundness;
    vec2 size;
    float border_thickness;
    float _;
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
in vec4 v_color;
in vec4 v_roundness;
in vec2 v_size;
out vec4 color;

float sd_rounded_box(in vec2 p, in vec2 b, in vec4 r)
{
    r.xy = (p.x>0.0)?r.xy : r.zw;
    r.x  = (p.y>0.0)?r.x  : r.y;
    vec2 q = abs(p)-b+r.x;
    return min(max(q.x,q.y),0.0) + length(max(q,0.0)) - r.x;
}

void main() 
{
    vec2 uv = v_tex_coord - 0.5;
    uv.x *= v_size.x;
    uv.y *= v_size.y;
    vec2 half_size = v_size * 0.5;
    float roundness = sd_rounded_box(uv, half_size, v_roundness);
    roundness = 1 - smoothstep(-2.0, 2.0, roundness);
    color = v_color * roundness;
}
