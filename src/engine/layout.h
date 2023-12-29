#pragma once

#include <core/defines.h>
#include <core/math.h>

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

//                                                         Center,  Bottom,   Top,  Right,  Left,  BottomLeft,  BottomRight,  TopLeft,  TopRight
const float32 AlignmentMultiplierX[AlignmentCount]     = {      0,       0,     0,   -0.5,   0.5,         0.5,         -0.5,      0.5,      -0.5 };
const float32 AlignmentMultiplierY[AlignmentCount]     = {      0,     0.5,  -0.5,      0,     0,         0.5,          0.5,     -0.5,      -0.5 };
const float32 AnchorMultiplierX[AlignmentCount]        = {      0,       0,     0,    0.5,  -0.5,        -0.5,          0.5,     -0.5,       0.5 };
const float32 AnchorMultiplierY[AlignmentCount]        = {      0,    -0.5,   0.5,      0,     0,        -0.5,         -0.5,      0.5,       0.5 };

/* Anchors */
const Anchor ANCHOR_BL_BL = { AlignmentBottomLeft, AlignmentBottomLeft };
const Anchor ANCHOR_BL_TR = { AlignmentBottomLeft, AlignmentTopRight };
const Anchor ANCHOR_BL_BR = { AlignmentBottomLeft, AlignmentBottomRight };
const Anchor ANCHOR_BL_TL = { AlignmentBottomLeft, AlignmentTopLeft };
const Anchor ANCHOR_TL_TL = { AlignmentTopLeft, AlignmentTopLeft };
const Anchor ANCHOR_TL_TR = { AlignmentTopLeft, AlignmentTopRight };
const Anchor ANCHOR_TL_BL = { AlignmentTopLeft, AlignmentBottomLeft };
const Anchor ANCHOR_TL_BR = { AlignmentTopLeft, AlignmentBottomRight };
const Anchor ANCHOR_BR_BR = { AlignmentBottomRight, AlignmentBottomRight };
const Anchor ANCHOR_BR_BL = { AlignmentBottomRight, AlignmentBottomLeft };
const Anchor ANCHOR_BR_TR = { AlignmentBottomRight, AlignmentTopRight };
const Anchor ANCHOR_BR_TL = { AlignmentBottomRight, AlignmentTopLeft };
const Anchor ANCHOR_TR_TR = { AlignmentTopRight, AlignmentTopRight };
const Anchor ANCHOR_TR_TL = { AlignmentTopRight, AlignmentTopLeft };
const Anchor ANCHOR_TR_BL = { AlignmentTopRight, AlignmentBottomLeft };
const Anchor ANCHOR_TR_BR = { AlignmentTopRight, AlignmentBottomRight };
const Anchor ANCHOR_T_TL = { AlignmentTop, AlignmentTopLeft };
const Anchor ANCHOR_T_TR = { AlignmentTop, AlignmentTopRight };
const Anchor ANCHOR_T_BL = { AlignmentTop, AlignmentBottomLeft };
const Anchor ANCHOR_T_BR = { AlignmentTop, AlignmentBottomRight };
const Anchor ANCHOR_T_T = { AlignmentTop, AlignmentTop };
const Anchor ANCHOR_T_L = { AlignmentTop, AlignmentLeft };
const Anchor ANCHOR_T_B = { AlignmentTop, AlignmentBottom };
const Anchor ANCHOR_T_R = { AlignmentTop, AlignmentRight };
const Anchor ANCHOR_L_TL = { AlignmentLeft, AlignmentTopLeft };
const Anchor ANCHOR_L_TR = { AlignmentLeft, AlignmentTopRight };
const Anchor ANCHOR_L_BL = { AlignmentLeft, AlignmentBottomLeft };
const Anchor ANCHOR_L_BR = { AlignmentLeft, AlignmentBottomRight };
const Anchor ANCHOR_L_T = { AlignmentLeft, AlignmentTop };
const Anchor ANCHOR_L_L = { AlignmentLeft, AlignmentLeft };
const Anchor ANCHOR_L_B = { AlignmentLeft, AlignmentBottom };
const Anchor ANCHOR_L_R = { AlignmentLeft, AlignmentRight };

typedef struct
{
	int32 columns;
	int32 rows;
	Vec2 padding;
	Rect base_container;
	Vec2 cell_size;
} LayoutGrid;

typedef struct
{
	Vec2 padding;
	Rect base_container;
	Rect row;
	float32 row_height;
	float32 spacing;
} LayoutStack;

typedef enum 
{
	CutSideLeft,
	CutSideRight,
	CutSideTop,
	CutSideBottom,
} CutSide;

typedef struct 
{
	Rect* rect;
	CutSide side;
} Cut;

internal Cut cut(Rect* rect, CutSide side);
internal Rect rect_cut_left(Rect* r, float32 size);
internal Rect rect_cut_right(Rect* r, float32 size);
internal Rect rect_cut_top(Rect* r, float32 size);
internal Rect rect_cut_bottom(Rect* r, float32 size);

internal Rect
rect_aligned(float32 x, float32 y, float32 w, float32 h, Alignment alignment);

internal Rect
rect_align(Rect rect, Alignment alignment);

internal Rect
rect_anchor(Rect child, Rect parent, Anchor anchor);

/** Similar to anchor but doesn't care about child x,y */
internal Rect 
rect_place(Rect child, Rect parent, Anchor anchor);

internal Rect
rect_place_under(Rect child, Rect parent);

internal Vec2
rect_relative(Rect rect, Alignment alignment);

internal Rect
rect_expand_f32(Rect rect, float32 v);

internal Rect
rect_expand(Rect rect, Vec2 v);

internal Rect
rect_shrink_f32(Rect rect, float32 v);

internal Rect
rect_shrink(Rect rect, Vec2 v);

internal Rect
rect_move(Rect rect, Vec2 v);

internal LayoutGrid
layout_grid(Rect container, int32 columns, int32 rows, Vec2 padding);

internal Rect
layout_grid_container(LayoutGrid layout);

internal Rect
layout_grid_cell(LayoutGrid layout, int32 column, int32 row);

internal Rect
layout_grid_multicell(LayoutGrid layout, int32 column, int32 row, int32 column_count, int32 row_count);

internal LayoutStack
layout_stack(Rect container, float row_height, Vec2 padding, float32 spacing);

internal Rect
layout_stack_push(LayoutStack* layout);

internal Rect
layout_stack_push_scaled(LayoutStack* layout, float32 scale);

internal Rect
layout_stack_container(LayoutStack* layout);

/** Rect Cut */
