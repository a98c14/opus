#include "main.h"

FT_Library library;
FT_Face    face;

#define GLYPH_METRICS_CAPACITY 128
#define FREE_GLYPH_FONT_SIZE   64

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
    logger_init();
    Arena*                 persistent_arena = make_arena_reserve(mb(128));
    Arena*                 frame_arena      = make_arena_reserve(mb(128));
    Window*                window           = window_create(persistent_arena, WINDOW_WIDTH, WINDOW_HEIGHT, "Scratch Window", NULL);
    RendererConfiguration* r_config         = r_config_new(frame_arena);
    r_config_set_screen_size(r_config, WINDOW_WIDTH, WINDOW_HEIGHT);
    r_config_set_world_size(r_config, 0, WINDOW_HEIGHT);
    r_config_set_clear_color(r_config, ColorBlack);
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

    FT_Int32 load_flags = FT_LOAD_RENDER | FT_LOAD_TARGET_(FT_RENDER_MODE_SDF);
    for (int i = 32; i < 128; ++i)
    {
        if (FT_Load_Char(face, i, load_flags))
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

    uint32 glyph_index = FT_Get_Char_Index(face, 'b');
    error              = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
    error              = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);

    if (error)
    {
        log_error("could not set font size");
    }
    TextureIndex texture = texture_new(g_renderer, atlas->atlas_width, atlas->atlas_height, 1, GL_LINEAR, NULL);

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
        atlas->metrics[i].tx = (float)x / (float)atlas->atlas_width;

        // glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexSubImage2D(GL_TEXTURE_2D, 0, x, 0, face->glyph->bitmap.width, face->glyph->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
        x += face->glyph->bitmap.width;
    }

    /* main loop */
    while (!glfwWindowShouldClose(window->glfw_window))
    {
        if (input_key_pressed(window, GLFW_KEY_RIGHT_BRACKET))
            break;

        draw_circle_filled(vec2_zero(), 100, ColorWhite);
        // draw_rect(rect_from_wh(200, 200), ColorWhite);

        r_render(g_renderer, time.dt);
        window_update(window);
    }

    window_destroy(window);
    logger_flush();
    return 0;
}