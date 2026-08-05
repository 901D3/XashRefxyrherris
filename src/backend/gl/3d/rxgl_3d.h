
#pragma once

typedef struct {
  // programs
  GLuint render3DVBO;

} rxgl_render3d_t;

extern rxgl_render3d_t glRender3D;

extern void GL_Render3D_SyncCvars( );

extern void GL_Render3D_ArenasInit( );
extern void GL_Render3D_ArenasShutdown( );

extern void GL_Render3D_RenderFlush( );

extern bool GL_Render3D_BaseInit( );
