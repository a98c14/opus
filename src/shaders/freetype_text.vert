#version 430 core

struct ShaderData
{
    vec4 glyph_bounds;
    vec4 color;
    vec4 outline_color;
    /** Softness of the glyph edges. Recommended value: 30 */
    float softness;
    /** Thickness of the glyph. Recommended value: 0.5, Values outside of the range 0.4 and 0.9 are not really usable */
    float thickness;
    /** Outline thickness. Should be between 0 and 0.5 */
    float outline_thickness;
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
out vec2 v_tex_coord;
flat out int v_instance_id;

void main() 
{
    gl_Position = projection * view * model[gl_InstanceID] * vec4(a_pos, 1.0);
    ShaderData v_data = data[gl_InstanceID];
    float w = v_data.glyph_bounds.z - v_data.glyph_bounds.x;
    float h = v_data.glyph_bounds.w - v_data.glyph_bounds.y;
    float x = ((a_tex_coord.x * w) + v_data.glyph_bounds.x) / texture_size.x;
    float y = (((1 - a_tex_coord.y) * h) + v_data.glyph_bounds.y) / texture_size.y;
    v_instance_id = gl_InstanceID;
    v_tex_coord = vec2(x, y);
}