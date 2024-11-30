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
out layout(location = 3) vec2 v_tex_coord;
out layout(location = 4) vec4 v_color;

void main() 
{
    v_tex_coord = a_tex_coord;
    v_color = data[gl_InstanceID].color;
    gl_Position = g_projection * g_view * data[gl_InstanceID].model * vec4(a_pos, 0, 1);
}