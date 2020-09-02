// c47edit - Scene editor for HM C47
// Copyright (C) 2018 AdrienTD
// Licensed under the GPL3+.
// See LICENSE file for more details.

#pragma once

#define WIN32_LEAN_AND_MEAN
#define _USE_MATH_DEFINES

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <vector>
#include <cstdint>
#include <cmath>

#ifndef APP_VERSION
#define APP_VERSION "DEV"
#endif

typedef unsigned int uint;

void ferr(char *str);
void warn(char *str);

// Unknown types that are guessed are marked with _temp

#define ZSTDOBJ 2

#define ZGATE_temp 21
#define ZBOUNDS_temp 28

#define ZROOM 33


#include "vecmat.h"
#include "chunk.h"
#include "gameobj.h"
#include "window.h"