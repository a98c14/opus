#pragma once
#define DEBUG_UI_ENABLED 0

#if DEBUG_UI_ENABLED == 1
#define debug_ui_rect(dc, rect, rotation, sort_index, style) draw_rect_internal(dc, rect, rotation, sort_index, style)
#else
#define debug_ui_rect(dc, rect, rotation, sort_index, style)
#endif