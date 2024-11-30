#version 430 core

layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_tex_coord;
layout(location = 2) in vec3 a_color;

struct ShaderData
{
    mat4 model;
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

/* Vertex Data */
in layout(location = 3) vec2 v_tex_coord;
in layout(location = 4) vec4 v_color;

out vec4 color;

float sd_equilateral_triangle( in vec2 p, in float r )
{
    const float k = sqrt(3.0);
    p.x = abs(p.x) - r;
    p.y = p.y + r/k;
    if( p.x+k*p.y>0.0 ) p = vec2(p.x-k*p.y,-k*p.x-p.y)/2.0;
    p.x -= clamp( p.x, -2.0*r, 0.0 );
    return -length(p)*sign(p.y);
}

float sd_triangle_isosceles( in vec2 p, in vec2 q )
{
    p.x = abs(p.x);
    vec2 a = p - q*clamp( dot(p,q)/dot(q,q), 0.0, 1.0 );
    vec2 b = p - q*vec2( clamp( p.x/q.x, 0.0, 1.0 ), 1.0 );
    float s = -sign( q.y );
    vec2 d = min( vec2( dot(a,a), s*(p.x*q.y-p.y*q.x) ),
                  vec2( dot(b,b), s*(p.y-q.y)  ));
    return -sqrt(d.x)*sign(d.y);
}

void main() 
{
    const float softness = 1.5;
    vec2 tri = vec2(1, -2);
    vec2 offset = vec2(0.0, -1.0);
    vec2 uv = v_tex_coord;
    uv = (uv * 2.0 - 1.0) + offset;

    vec2 p = fwidth(uv);
    float a = -sd_triangle_isosceles(uv, tri);
    float a_x = smoothstep(0.0, softness*p.x, a);
    float a_y = smoothstep(0.0, softness*p.y, a);
    a = min(a_x, a_y) * max(a_x, a_y);
    
    color = vec4(v_color.rgb, a * v_color.a);
}
