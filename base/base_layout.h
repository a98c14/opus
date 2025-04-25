#pragma once
#include "base_asserts.h"
#include "base_defines.h"
#include "base_math.h"

typedef enum
{
    AlignmentCenter,
    AlignmentBottom,
    AlignmentTop,
    AlignmentRight,
    AlignmentLeft,

    AlignmentBottomLeft,
    AlignmentBottomRight,
    AlignmentTopLeft,
    AlignmentTopRight,
    AlignmentCount
} Alignment;

typedef struct
{
    Alignment parent;
    Alignment child;
} Anchor;

//                                                   Center,  Bottom,   Top,  Right,  Left,  BottomLeft,  BottomRight,  TopLeft,  TopRight
const float32 AlignmentMultiplierX[AlignmentCount] = {0, 0, 0, -0.5, 0.5, 0.5, -0.5, 0.5, -0.5};
const float32 AlignmentMultiplierY[AlignmentCount] = {0, 0.5, -0.5, 0, 0, 0.5, 0.5, -0.5, -0.5};
const float32 AnchorMultiplierX[AlignmentCount]    = {0, 0, 0, 0.5, -0.5, -0.5, 0.5, -0.5, 0.5};
const float32 AnchorMultiplierY[AlignmentCount]    = {0, -0.5, 0.5, 0, 0, -0.5, -0.5, 0.5, 0.5};

/* Anchors */
const Anchor ANCHOR_BL_BL = {AlignmentBottomLeft, AlignmentBottomLeft};
const Anchor ANCHOR_BL_TR = {AlignmentBottomLeft, AlignmentTopRight};
const Anchor ANCHOR_BL_BR = {AlignmentBottomLeft, AlignmentBottomRight};
const Anchor ANCHOR_BL_TL = {AlignmentBottomLeft, AlignmentTopLeft};
const Anchor ANCHOR_TL_TL = {AlignmentTopLeft, AlignmentTopLeft};
const Anchor ANCHOR_TL_TR = {AlignmentTopLeft, AlignmentTopRight};
const Anchor ANCHOR_TL_BL = {AlignmentTopLeft, AlignmentBottomLeft};
const Anchor ANCHOR_TL_BR = {AlignmentTopLeft, AlignmentBottomRight};
const Anchor ANCHOR_BR_BR = {AlignmentBottomRight, AlignmentBottomRight};
const Anchor ANCHOR_BR_BL = {AlignmentBottomRight, AlignmentBottomLeft};
const Anchor ANCHOR_BR_TR = {AlignmentBottomRight, AlignmentTopRight};
const Anchor ANCHOR_BR_TL = {AlignmentBottomRight, AlignmentTopLeft};
const Anchor ANCHOR_BR_C  = {AlignmentBottomRight, AlignmentCenter};
const Anchor ANCHOR_TR_TR = {AlignmentTopRight, AlignmentTopRight};
const Anchor ANCHOR_TR_TL = {AlignmentTopRight, AlignmentTopLeft};
const Anchor ANCHOR_TR_BL = {AlignmentTopRight, AlignmentBottomLeft};
const Anchor ANCHOR_TR_BR = {AlignmentTopRight, AlignmentBottomRight};
const Anchor ANCHOR_T_TL  = {AlignmentTop, AlignmentTopLeft};
const Anchor ANCHOR_T_TR  = {AlignmentTop, AlignmentTopRight};
const Anchor ANCHOR_T_BL  = {AlignmentTop, AlignmentBottomLeft};
const Anchor ANCHOR_T_BR  = {AlignmentTop, AlignmentBottomRight};
const Anchor ANCHOR_T_T   = {AlignmentTop, AlignmentTop};
const Anchor ANCHOR_T_L   = {AlignmentTop, AlignmentLeft};
const Anchor ANCHOR_T_B   = {AlignmentTop, AlignmentBottom};
const Anchor ANCHOR_T_R   = {AlignmentTop, AlignmentRight};
const Anchor ANCHOR_B_TL  = {AlignmentBottom, AlignmentTopLeft};
const Anchor ANCHOR_B_TR  = {AlignmentBottom, AlignmentTopRight};
const Anchor ANCHOR_B_BL  = {AlignmentBottom, AlignmentBottomLeft};
const Anchor ANCHOR_B_BR  = {AlignmentBottom, AlignmentBottomRight};
const Anchor ANCHOR_B_T   = {AlignmentBottom, AlignmentTop};
const Anchor ANCHOR_B_L   = {AlignmentBottom, AlignmentLeft};
const Anchor ANCHOR_B_B   = {AlignmentBottom, AlignmentBottom};
const Anchor ANCHOR_B_R   = {AlignmentBottom, AlignmentRight};
const Anchor ANCHOR_B_C   = {AlignmentBottom, AlignmentCenter};
const Anchor ANCHOR_L_TL  = {AlignmentLeft, AlignmentTopLeft};
const Anchor ANCHOR_L_TR  = {AlignmentLeft, AlignmentTopRight};
const Anchor ANCHOR_L_BL  = {AlignmentLeft, AlignmentBottomLeft};
const Anchor ANCHOR_L_BR  = {AlignmentLeft, AlignmentBottomRight};
const Anchor ANCHOR_L_T   = {AlignmentLeft, AlignmentTop};
const Anchor ANCHOR_L_L   = {AlignmentLeft, AlignmentLeft};
const Anchor ANCHOR_L_B   = {AlignmentLeft, AlignmentBottom};
const Anchor ANCHOR_L_R   = {AlignmentLeft, AlignmentRight};
const Anchor ANCHOR_R_TL  = {AlignmentRight, AlignmentTopLeft};
const Anchor ANCHOR_R_TR  = {AlignmentRight, AlignmentTopRight};
const Anchor ANCHOR_R_BL  = {AlignmentRight, AlignmentBottomLeft};
const Anchor ANCHOR_R_BR  = {AlignmentRight, AlignmentBottomRight};
const Anchor ANCHOR_R_T   = {AlignmentRight, AlignmentTop};
const Anchor ANCHOR_R_L   = {AlignmentRight, AlignmentLeft};
const Anchor ANCHOR_R_B   = {AlignmentRight, AlignmentBottom};
const Anchor ANCHOR_R_R   = {AlignmentRight, AlignmentRight};
const Anchor ANCHOR_C_C   = {AlignmentCenter, AlignmentCenter};
const Anchor ANCHOR_C_TL  = {AlignmentCenter, AlignmentTopLeft};
const Anchor ANCHOR_C_TR  = {AlignmentCenter, AlignmentTopRight};
const Anchor ANCHOR_C_BL  = {AlignmentCenter, AlignmentBottomLeft};
const Anchor ANCHOR_C_BR  = {AlignmentCenter, AlignmentBottomRight};
const Anchor ANCHOR_C_T   = {AlignmentCenter, AlignmentTop};
const Anchor ANCHOR_C_L   = {AlignmentCenter, AlignmentLeft};
const Anchor ANCHOR_C_B   = {AlignmentCenter, AlignmentBottom};
const Anchor ANCHOR_C_R   = {AlignmentCenter, AlignmentRight};

typedef enum
{
    CutSideLeft,
    CutSideRight,
    CutSideTop,
    CutSideBottom,
} CutSide;

typedef struct
{
    Rect*   rect;
    CutSide side;
} Cut;

internal Cut  cut(Rect* rect, CutSide side);
internal Rect rect_cut(Rect* r, float32 size, CutSide side);
internal Rect rect_cut_r(Rect* r, Rect size, CutSide side);
internal Rect rect_cut_left(Rect* r, float32 size);
internal Rect rect_cut_right(Rect* r, float32 size);
internal Rect rect_cut_top(Rect* r, float32 size);
internal Rect rect_cut_bottom(Rect* r, float32 size);

internal Rect rect_at(Vec2 position, Vec2 size, Alignment alignment);
internal Rect rect_aligned(float32 x, float32 y, float32 w, float32 h, Alignment alignment);
internal Vec2 rect_get(Rect rect, Alignment alignment);
internal Rect rect_align(Rect rect, Alignment alignment);
internal Rect rect_anchor(Rect child, Rect parent, Anchor anchor);
internal Rect rect_at_o(Vec2 position, Vec2 size, Vec2 offset, Alignment alignment);

/** Similar to anchor but doesn't care about child x,y */
internal Rect rect_place(Rect child, Rect parent, Anchor anchor);
internal Rect rect_place_under(Rect child, Rect parent);
internal Vec2 rect_relative(Rect rect, Alignment alignment);
internal Rect rect_expand_f32(Rect rect, float32 v);
internal Rect rect_expand(Rect rect, Vec2 v);
internal Rect rect_shrink_f32(Rect rect, float32 v);
internal Rect rect_shrink(Rect rect, Vec2 v);
internal Rect rect_move(Rect rect, Vec2 v);
internal Rect rect_resize(Rect r, float32 w, float32 h, Alignment alignment);
internal Rect rect_resize_height(Rect r, float32 h, Alignment alignment);
internal Rect rect_resize_width(Rect r, float32 w, Alignment alignment);
