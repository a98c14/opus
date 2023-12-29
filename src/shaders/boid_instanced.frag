#version 430 core

struct ShaderData
{
    vec4 color;
};

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

layout (std140, binding = 3) buffer Matrices
{
    mat4 model[];
};

layout (std140, binding = 4) buffer Custom
{
    ShaderData data[];
};

uniform sampler2D u_main_texture;

/* Vertex Data */
in vec2 v_tex_coord;
flat in int v_instance_id;

out vec4 color;

float sd_triangle_isosceles(in vec2 p, in vec2 q)
{
    p.x = abs(p.x);
    vec2 a = p - q*clamp( dot(p,q)/dot(q,q), 0.0, 1.0 );
    vec2 b = p - q*vec2( clamp( p.x/q.x, 0.0, 1.0 ), 1.0 );
    float s = -sign( q.y );
    vec2 d = min(vec2(dot(a,a), s*(p.x*q.y-p.y*q.x) ),
                 vec2(dot(b,b), s*(p.y-q.y)));
    return -sqrt(d.x)*sign(d.y);
}

void main() {
    float smoothness = 5;
    vec2 dxy = fwidth(v_tex_coord);
    vec2 uv = v_tex_coord;
    uv = uv * 2.0 - 1.0;
    uv = uv + vec2(0.0, 0.8);
    float d = sd_triangle_isosceles(uv, vec2(0.8, 1.6));
    d = smoothstep(0.0, dxy.x * smoothness, d);
    d = 1 - d;
    color = vec4(data[v_instance_id].color.xyz, d * data[v_instance_id].color.a);
}