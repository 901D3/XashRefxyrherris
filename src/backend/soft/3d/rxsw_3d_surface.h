
#pragma once

#include "com_model.h"
#include "cl_entity.h"

#include "rx_image.h"
#include "rx_3d_surface.h"

extern float surfaceNearZ;

extern void Soft_Render3D_Surface_RenderAllSurfaces( );
extern void Soft_Render3D_Surface_ProcessSurfaceGeneric( rx_surfaceargs_t* rxSurface, float* transfVertices, float** origVertices, int vertexCount );

extern void Soft_Render3D_SurfaceArena_Shutdown( );
extern void Soft_Render3D_SurfaceArena_Init( );
extern void Soft_Render3D_SurfaceArena_InitInvzBuffer( );
