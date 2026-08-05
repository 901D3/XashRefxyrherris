
#pragma once

extern void GL_TriAPI_SetBlendMode(int blendMode);
extern void GL_TriAPI_Immediate_Begin(int drawMode);
extern void GL_TriAPI_Immediate_End();
extern void GL_TriAPI_CullFace(int cullMode);

extern void GL_TriAPI_ArenaInit();

extern bool GL_TriAPI_Init();
