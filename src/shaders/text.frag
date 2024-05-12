#version 430 core
#define DEBUG 0

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
in vec2 v_tex_coord;
flat in int v_instance_id;

out vec4 color;

void main() {
    ShaderData v_data = data[v_instance_id];
    vec2 uv = v_tex_coord;
    vec2 dxy = fwidth(uv);
    float d = texture(u_main_texture, uv).r;
    float softness = v_data.softness * max(dxy.x, dxy.y);
    float alpha = smoothstep(1 - v_data.thickness - softness, 1 - v_data.thickness + softness, d);
    float outline_alpha = smoothstep(1 - v_data.thickness - softness - v_data.outline_thickness, 1 - v_data.thickness + softness - v_data.outline_thickness, d);
    vec4 outline_color = vec4(v_data.outline_color.xyz, outline_alpha * v_data.outline_color.a);
    vec4 inside_color = vec4(v_data.color.xyz, alpha * v_data.color.a);
    color = mix(inside_color, outline_color, clamp(outline_color.a - alpha, 0, 1));
#if DEBUG == 1
    color = mix(vec4(1, 0, 0, 1), color, color.a);
#endif
}