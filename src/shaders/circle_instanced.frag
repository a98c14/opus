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
    float softness = 3;
    vec2 uv = v_tex_coord;
    uv = uv * 2.0 - 1.0;

    // partial circle
    // TODO: maybe use different shader for this? might be 
    // to costly for circles that don't use this feature.
    float angle = acos(dot(normalize(uv), vec2(1, 0))) / 3.1415926;
    float enabled = 1 - step(v_data.slice, angle);

    vec2 dp = fwidth(uv);
    softness *= dp.x;
    float d = smoothstep((1 - v_data.fill_ratio) - softness * 2, (1 - v_data.fill_ratio) - softness, length(uv)) - smoothstep(1 - softness, 1.0, length(uv));
    color = vec4(v_data.color.xyz, d * v_data.color.a) * enabled;
}