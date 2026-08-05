
#pragma once

#include <stdint.h>

extern void Soft_TriAPI_SetBlendMode(int blendMode);
extern void Soft_TriAPI_Immediate_Begin(int drawMode);
extern void Soft_TriAPI_Immediate_End();
extern void Soft_TriAPI_CullFace(int cullMode);

extern bool Soft_TriAPI_Init();
