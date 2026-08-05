
#pragma once

#include "rx_image.h"

/*
## Blit.cpp
*/
extern void Soft_Video_BlitScreen();
extern void Soft_Video_BlitToScreenNoRotate();

/*
## Video.cpp
*/
extern void Soft_Video_Set2DMode(int enable);

extern bool Soft_Video_EngineCreateBuffer();

extern void Soft_Video_AllocateScreenBuffer();

extern void Soft_Video_OnViewChange();

extern void Soft_Video_Shutdown();
extern bool Soft_Video_Init();
