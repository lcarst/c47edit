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

#define NUM_ZTYPES	256	// Maximum encountered is 255
// Unknown types that are guesses are marked with _<number>
// 0 0x00
#define Z0				0
#define ZGROUP			1
#define ZSTDOBJ			2
#define ZCAMERA			3
#define ZGIGAOBJ		8
#define ZEFFECT_11		11
#define ZENVIRONMENT	13

// 16 0x10
#define ZPICOBJ			17
#define ZSNDOBJ			18
#define ZGATE_21		21
#define ZSHOTOBJ		23
#define ZLIST			26
#define ZBOUNDS_28		28

// 32 0x20
#define ZROOM			33
#define ZSHAPE			34
#define ZSPOTLIGHT		35
#define ZOMNILIGHT		36
#define ZIKNLOBJ		39
#define ZWINOBJ			44
#define ZCHAROBJ		45
#define ZWINGROUP		46
#define ZFONT			47

// 48 0x30
#define ZWINDOWS		48
#define ZWINDOW			49
#define ZBUTTON			51
#define ZLINEOBJ		56
#define ZTTFONT			58
#define ZSCROLLAREA		59

// 64 0x40
#define ZSCROLLBAR		64
#define ZSCALESTDOBJ	65

// 80 ox50

// 96 0x60
#define ZITEMGROUP		102
#define ZITEMGROUPWEAPON	105
#define ZITEMGROUPAMMO	106
#define ZPATHFINDER2	111

// 112 0x70
#define ZDOOR_112		112

// 200
#define ZCONDITIONLIST	200
#define ZACTIONARBITER	201
#define ZBOXPRIMITIVE	225






#define DEG_TO_RAD M_PI / 180f
#define RAD_TO_DEG 180f / M_PI

#include "vecmat.h"
#include "utils.h"
#include "chunk.h"
#include "gameobj.h"
#include "window.h"