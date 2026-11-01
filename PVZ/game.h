#pragma once

#include <graphics.h>
#include <stdio.h>
#include <stdlib.h>
#include <mmsystem.h>
#include "vector2.h"
#include "tools.h"

#define WIN_WIDTH 900
#define WIN_HEIGHT 600

enum { WAN_DOU, XIANG_RI_KUI, DA_ZUI_HUA, TU_DOU_DI_LEI, JIAN_GUO_QIANG, ZHI_WU_COUNT };
enum { SUNSHINE_DOWN, SUNSHINE_GROUND, SUNSHINE_COLLET, SUNHSINE_PRODUCT };

// ---- 全局资源声明（定义在 main.cpp）----
extern IMAGE imgBg;
extern IMAGE imaBar;
extern IMAGE imgCards[ZHI_WU_COUNT];
extern IMAGE* imgZhiWu[ZHI_WU_COUNT][20];
extern IMAGE imgSunshineBall[29];
extern IMAGE imgZombies[22];
extern IMAGE imgBulletNormal;
extern IMAGE imgBulletBlast[4];
extern IMAGE imgZombieDead[20];
extern IMAGE imgZombieEat[21];
extern IMAGE imgChomperEat[9];
extern IMAGE imgChomperDigest[6];
extern IMAGE imgPotatoDown;
extern IMAGE imgPotatoBlast;

extern int curX, curY;
extern int curZhiWu;
extern int sunshine;
extern int plantCost[ZHI_WU_COUNT];
