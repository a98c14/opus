#include "main.h"

FT_Library library;
FT_Face    face;

#define GLYPH_METRICS_CAPACITY 128
#define FREE_GLYPH_FONT_SIZE   16

typedef struct
{
    float ax; // advance.x
    float ay; // advance.y

    float bw; // bitmap.width;
    float bh; // bitmap.rows;

    float bl; // bitmap_left;
    float bt; // bitmap_top;

    float tx; // x offset of glyph in texture coordinates
} FreeGlyphMetric;

typedef struct
{
    FT_UInt         atlas_width;
    FT_UInt         atlas_height;
    GLuint          glyphs_texture;
    FreeGlyphMetric metrics[GLYPH_METRICS_CAPACITY];
} FreeGlyphAtlas;

int
main(void)
{
    /*  initialization */
    ThreadContext tctx;
    tctx_init_and_equip(&tctx);
    logger_init();
    Arena*                 persistent_arena = make_arena_reserve(mb(128));
    Arena*                 frame_arena      = make_arena_reserve(mb(128));
    Window*                window           = window_create(persistent_arena, WINDOW_WIDTH, WINDOW_HEIGHT, "Scratch Window", NULL);
    RendererConfiguration* r_config         = r_config_new(frame_arena);
    r_config_set_screen_size(r_config, WINDOW_WIDTH, WINDOW_HEIGHT);
    r_config_set_world_size(r_config, 0, WINDOW_HEIGHT);
    r_config_set_clear_color(r_config, ColorSlate900);
    renderer_init(persistent_arena, r_config);

    R_PipelineConfiguration* config       = r_pipeline_config_new(frame_arena);
    PassIndex                pass_default = r_pipeline_config_add_pass(config, FRAME_BUFFER_INDEX_DEFAULT);
    r_pipeline_init(config);
    draw_context_init(persistent_arena, frame_arena, g_renderer, pass_default);
    EngineTime time = engine_time_new();

    /** FREE TYPE, Will be moved to `opus` */
    int32 error = FT_Init_FreeType(&library);
    if (error)
    {
        log_error("could not initialize freetype");
    }

    error = FT_New_Face(library, ASSET_PATH "\\open_sans.ttf", 0, &face);
    if (error == FT_Err_Unknown_File_Format)
    {
        log_error("could not load font face, unknown format");
    }
    else if (error)
    {
        log_error("could not load font face");
    }

    FreeGlyphAtlas* atlas = arena_push_struct_zero(persistent_arena, FreeGlyphAtlas);

    FT_UInt pixel_size = FREE_GLYPH_FONT_SIZE;
    error              = FT_Set_Pixel_Sizes(face, 0, pixel_size);
    if (error)
    {
        fprintf(stderr, "ERROR: Could not set pixel size to %u\n", pixel_size);
        return 1;
    }

    // FT_Int32 load_flags = FT_LOAD_RENDER | FT_LOAD_TARGET_(FT_RENDER_MODE_SDF);
    FT_Int32 load_flags = FT_LOAD_RENDER;
    for (int i = 32; i < 128; ++i)
    {
        if (FT_Load_Char(face, i, FT_LOAD_DEFAULT))
        {
            fprintf(stderr, "ERROR: could not load glyph of a character with code %d\n", i);
            exit(1);
        }

        atlas->atlas_width += face->glyph->bitmap.width;
        if (atlas->atlas_height < face->glyph->bitmap.rows)
        {
            atlas->atlas_height = face->glyph->bitmap.rows;
        }
    }

    ArenaTemp     temp               = scratch_begin(0, 0);
    TextureIndex  texture            = texture_new(g_renderer, atlas->atlas_width, atlas->atlas_height, 1, GL_LINEAR, NULL);
    MaterialIndex free_type_material = material_new(
        g_renderer,
        file_read_all_as_string(temp.arena, string(SHADER_PATH "\\freetype_text.vert")),
        file_read_all_as_string(temp.arena, string(SHADER_PATH "\\freetype_text.frag")),
        sizeof(ShaderDataText),
        true);

    int x = 0;
    for (int i = 32; i < 128; ++i)
    {
        if (FT_Load_Char(face, i, load_flags))
        {
            fprintf(stderr, "ERROR: could not load glyph of a character with code %d\n", i);
            exit(1);
        }

        if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL))
        {
            fprintf(stderr, "ERROR: could not render glyph of a character with code %d\n", i);
            exit(1);
        }

        atlas->metrics[i].ax = face->glyph->advance.x >> 6;
        atlas->metrics[i].ay = face->glyph->advance.y >> 6;
        atlas->metrics[i].bw = face->glyph->bitmap.width;
        atlas->metrics[i].bh = face->glyph->bitmap.rows;
        atlas->metrics[i].bl = face->glyph->bitmap_left;
        atlas->metrics[i].bt = face->glyph->bitmap_top;
        atlas->metrics[i].tx = x;
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexSubImage2D(GL_TEXTURE_2D, 0, x, 0, face->glyph->bitmap.width, face->glyph->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
        x += face->glyph->bitmap.width;
    }

    /* main loop */
    while (!glfwWindowShouldClose(window->glfw_window))
    {
        arena_reset(frame_arena);
        if (input_key_pressed(window, GLFW_KEY_RIGHT_BRACKET))
            break;

        // draw_circle_filled(vec2_zero(), 100, ColorWhite);
        // draw_rect(rect_from_wh(200, 200), ColorWhite);

        String test_string = string("Do small fonts look ok? ggg");
        String english     = string("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890~!@#$%^&*()-_+=[{]}\\|;:'\",<.>/?");

        ShaderDataText shader_data    = {0};
        shader_data.color             = d_color_white;
        shader_data.thickness         = d_default_text_thickness;
        shader_data.softness          = d_default_text_softness;
        shader_data.outline_thickness = d_default_text_outline_thickness;

        float32 base_x = -200, base_y = 0;

        RenderKey key   = render_key_new(d_state->ctx->view, d_state->ctx->sort_layer, d_state->ctx->pass, texture, g_renderer->quad, free_type_material);
        R_Batch*  batch = r_batch_from_key(key, english.length);

        draw_line(vec2(base_x, base_y), vec2(200, 0), ColorRed500, 3);
        draw_text(english, rect_at(vec2(base_x, 50), vec2(200, 0), AlignmentBottomLeft), ANCHOR_BL_BL, 14, ColorWhite100);

        Mat4*           model_data         = batch->model_buffer;
        ShaderDataText* shader_data_buffer = (ShaderDataText*)batch->uniform_buffer;
        Vec2            position           = vec2_zero();
        float32         sx                 = 1;
        float32         sy                 = 1;
        for (uint32 i = 0; i < english.length; i++)
        {
            FreeGlyphMetric glyph = atlas->metrics[(uint8)english.value[i]];
            float32         x2    = base_x + glyph.bl * sx;
            float32         y2    = base_y + (glyph.bt - glyph.bh) * sy;
            float32         w     = glyph.bw * sx;
            float32         h     = glyph.bh * sy;

            /* Advance the cursor to the start of the next character */
            base_x += glyph.ax * sx;
            base_y += glyph.ay * sy;

            /* Skip glyphs that have no pixels */
            if (!w || !h)
                continue;

            shader_data.glyph_bounds.x = glyph.tx;
            shader_data.glyph_bounds.y = 0;
            shader_data.glyph_bounds.z = glyph.tx + glyph.bw;
            shader_data.glyph_bounds.w = glyph.bh;
            memcpy(&shader_data_buffer[i], &shader_data, sizeof(ShaderDataText));

#if DEBUG_TEXT
            draw_debug_rect(string_bounds);
#endif
            float32 x = position.x + x2 + w / 2.0f;
            float32 y = position.y + y2 + h / 2.0f;

            Mat4 transform = transform_quad_aligned(vec2(x, y), vec2(w, h));
            memcpy(&model_data[i], &transform, sizeof(transform));
        }

        r_render(g_renderer, time.dt);
        window_update(window);
    }

    window_destroy(window);
    logger_flush();
    return 0;
}