#ifndef PHYBAL_H
#define PHYBAL_H

#include <windows.h>
#include <math.h>
#include <stdio.h>
#include "Ball.h"
#include "Block.h"

#pragma comment(lib, "msimg32.lib");

#define sign(x) (x>=0?1:-1)

#ifdef EXT_GLOBAL //헤더에 전역변수를 선언하기 위한 트릭
#define EXT
#else
#define EXT extern
#endif


#endif