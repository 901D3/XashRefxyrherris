
#pragma once

typedef struct {
  float rotatedPos[ 3 ];
} rxsw_render3d_t;

extern rxsw_render3d_t swRender3D;

extern void Soft_Render3D_SyncCvars( );
extern void Soft_Render3D_ArenasInit( );
extern void Soft_Render3D_ArenasShutdown( );
extern void Soft_Render3D_RenderFlush( );
extern void Soft_Render3D_BaseInit( );
