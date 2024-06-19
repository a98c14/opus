#pragma once

#include "ui_core.h"
// i don't like that we have to include this
// can we move the intersection code to `base` layer?
#include <physics/intersection.h>

internal bool32 ui_button(UI_Key key, String label);
