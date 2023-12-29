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
    vec4 u_color;
};

uniform mat4 u_model;
uniform sampler2D u_main_texture;

/* Vertex Data */
in vec2 v_tex_coord;

out vec4 color;

float sdEquilateralTriangle(in vec2 p, in float r)
{
    const float k = sqrt(3.0);
    p.x = abs(p.x) - r;
    p.y = p.y + r/k;
    if(p.x+k*p.y>0.0) p=vec2(p.x-k*p.y,-k*p.x-p.y)/2.0;
    p.x -= clamp(p.x,-2.0*r, 0.0 );
    return -length(p)*sign(p.y);
}

void main() {
    vec2 uv = v_tex_coord;
    uv = uv * 2 - 1;
    uv.y += 0.15;
    float d = sdEquilateralTriangle(uv, 1.0f);
	vec3 col = vec3(1.0-smoothstep(0.0,0.02,abs(d)));

    color = vec4(col, 1.0f);
}
