#include "gfx_utils.h"

/** resource load utility */
internal MaterialIndex
gfx_material_from_path(String vertex_shader_path, String frag_shader_path, uint32 uniform_data_size, GFX_DrawType draw_type)
{
    ArenaTemp     temp          = scratch_begin(0, 0);
    String        vertex_shader = os_path_read_all_as_string(vertex_shader_path, temp.arena);
    String        frag_shader   = os_path_read_all_as_string(frag_shader_path, temp.arena);
    MaterialIndex result        = gfx_material_new(vertex_shader, frag_shader, uniform_data_size, draw_type);
    return result;
}
