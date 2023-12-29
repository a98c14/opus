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

// uniform mat4 u_mvp;
uniform sampler2D u_main_texture;

/* Vertex Data */
out vec2 v_tex_coord;
flat out int v_instance_id;

void main() 
{
    gl_Position = projection * view * model[gl_InstanceID] * vec4(a_pos, 1.0);
    v_instance_id = gl_InstanceID;
    v_tex_coord = a_tex_coord;
}