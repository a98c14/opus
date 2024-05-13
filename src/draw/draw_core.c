#include "draw_core.h"

internal void
d_context_init(Arena* persistent_arena)
{
    d_context = arena_push_struct_zero(persistent_arena, D_Context);

    ArenaTemp            temp      = scratch_begin(0, 0);
    VertexAttributeInfo* attr_info = r_attribute_info_new(temp.arena);
    r_attribute_info_add_vec2(attr_info); // layout(location = 0) in vec2 a_pos;
    r_attribute_info_add_vec2(attr_info); // layout(location = 1) in vec2 a_tex_coord;

    VertexAttributeInfo* attr_info_color = r_attribute_info_new(temp.arena);
    r_attribute_info_add_vec2(attr_info_color); // layout(location = 0) in vec2 a_pos;
    r_attribute_info_add_vec2(attr_info_color); // layout(location = 1) in vec2 a_tex_coord;
    r_attribute_info_add_vec4(attr_info_color); // layout(location = 2) in vec4 a_color;

    VertexAttributeInfo* attr_info_color_instanced = r_attribute_info_new(temp.arena);
    r_attribute_info_add_vec2(attr_info_color_instanced); // layout(location = 0) in vec2 a_pos;
    r_attribute_info_add_vec2(attr_info_color_instanced); // layout(location = 1) in vec2 a_tex_coord;
    r_attribute_info_add_vec4(attr_info_color_instanced); // layout(location = 2) in vec4 a_color;
    r_attribute_info_add_vec4(attr_info_color_instanced); // layout(location = 3) in int a_instance;

    d_context->material_text = r_material_create(
        g_renderer,
        d_shader_opengl_font_vert,
        d_shader_opengl_font_frag,
        16,
        false, attr_info);

    d_context->material_sprite = r_material_create(
        g_renderer,
        d_shader_opengl_sprite_vert,
        d_shader_opengl_sprite_frag,
        16,
        false, attr_info);

    d_context->material_basic = r_material_create(
        g_renderer,
        d_shader_opengl_basic_vert,
        d_shader_opengl_basic_frag,
        16,
        false, attr_info_color);

    d_context->material_rect = r_material_create(
        g_renderer,
        d_shader_opengl_rect_vert,
        d_shader_opengl_rect_frag,
        16,
        false, attr_info_color);

    d_context->material_circle = r_material_create(
        g_renderer,
        d_shader_opengl_circle_vert,
        d_shader_opengl_circle_frag,
        16,
        false, attr_info_color_instanced);

    d_context->active_pass  = 0;
    d_context->active_layer = 5;

    scratch_end(temp);
}

internal void
d_batch_push_vertex_pos_tex_color(Vec2 pos, Vec2 tex_coord, Color c)
{
    VertexAtrribute_TexturedColored attr;
    attr.pos       = pos;
    attr.tex_coord = tex_coord;
    attr.color     = color_v4(c);

    R_Batch* batch = r_active_batch();
    memcpy((uint8*)batch->vertex_buffer + batch->vertex_buffer_size, &attr, sizeof(attr));
    batch->vertex_buffer_size += sizeof(attr);
    batch->vertex_count++;
}

internal void
d_batch_push_vertex_pos_tex_color_instanced(Vec2 pos, Vec2 tex_coord, Color c, int32 instance)
{
    VertexAtrribute_TexturedColoredInstanced attr;
    attr.pos       = pos;
    attr.tex_coord = tex_coord;
    attr.color     = color_v4(c);
    attr.instance  = instance;

    R_Batch* batch = r_active_batch();
    memcpy((uint8*)batch->vertex_buffer + batch->vertex_buffer_size, &attr, sizeof(attr));
    batch->vertex_buffer_size += sizeof(attr);
    batch->vertex_count++;
}

internal void
d_batch_push_colored_quad(Rect rect, Bounds tex_coord, Color c)
{
    R_Batch* batch = r_active_batch();
    d_batch_push_vertex_pos_tex_color(rect_bl(rect), bounds_bl(tex_coord), c);
    d_batch_push_vertex_pos_tex_color(rect_tl(rect), bounds_tl(tex_coord), c);
    d_batch_push_vertex_pos_tex_color(rect_br(rect), bounds_br(tex_coord), c);
    d_batch_push_vertex_pos_tex_color(rect_br(rect), bounds_br(tex_coord), c);
    d_batch_push_vertex_pos_tex_color(rect_tl(rect), bounds_tl(tex_coord), c);
    d_batch_push_vertex_pos_tex_color(rect_tr(rect), bounds_tr(tex_coord), c);
    batch->element_count++;
}

/** draw functions */
internal void
d_line(Vec2 start, Vec2 end, float32 thickness, Color c)
{
    RenderKey test_key = render_key_new_default(ViewTypeWorld, d_context->active_layer, d_context->active_pass, 0, d_context->material_rect);

    Vec2 heading = heading_to_vec2(start, end);
    Vec2 A1      = add_vec2(start, mul_vec2_f32(rotate90_vec2(heading), thickness));
    Vec2 A2      = add_vec2(start, mul_vec2_f32(rotate90i_vec2(heading), thickness));
    Vec2 B1      = add_vec2(end, mul_vec2_f32(rotate90_vec2(heading), thickness));
    Vec2 B2      = add_vec2(end, mul_vec2_f32(rotate90i_vec2(heading), thickness));

    r_batch_scope(test_key)
    {
        d_batch_push_vertex_pos_tex_color(A1, vec2(0, 0), c);
        d_batch_push_vertex_pos_tex_color(A2, vec2(0, 1), c);
        d_batch_push_vertex_pos_tex_color(B1, vec2(1, 0), c);
        d_batch_push_vertex_pos_tex_color(B1, vec2(1, 0), c);
        d_batch_push_vertex_pos_tex_color(A2, vec2(0, 1), c);
        d_batch_push_vertex_pos_tex_color(B2, vec2(1, 1), c);
    }
}

internal void
d_circle(Vec2 pos, float32 radius, float32 thickness, Color c)
{
    RenderKey test_key = render_key_new_default(ViewTypeWorld, d_context->active_layer, d_context->active_pass, 0, d_context->material_circle);

    Rect r = rect_at(pos, vec2(radius, radius), AlignmentCenter);

    r_batch_scope(test_key)
    {
        R_Batch* batch    = r_active_batch();
        int32    instance = batch->element_count;

        d_batch_push_vertex_pos_tex_color_instanced(rect_bl(r), vec2(0, 0), c, instance);
        d_batch_push_vertex_pos_tex_color_instanced(rect_tl(r), vec2(0, 1), c, instance);
        d_batch_push_vertex_pos_tex_color_instanced(rect_br(r), vec2(1, 0), c, instance);
        d_batch_push_vertex_pos_tex_color_instanced(rect_br(r), vec2(1, 0), c, instance);
        d_batch_push_vertex_pos_tex_color_instanced(rect_tl(r), vec2(0, 1), c, instance);
        d_batch_push_vertex_pos_tex_color_instanced(rect_tr(r), vec2(1, 1), c, instance);

        TempCircleShaderData d = {0};
        d.thickness            = thickness;

        memcpy((uint8*)batch->uniform_buffer + batch->uniform_buffer_size, &d, sizeof(TempCircleShaderData));
        batch->uniform_buffer_size += sizeof(TempCircleShaderData);
        batch->element_count++;
    }
}