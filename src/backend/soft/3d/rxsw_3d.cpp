
#include "rx_base.h"

#include "rx_3d.h"

#include "rxsw_3d.h"
#include "rxsw_3d_surface.h"

#include "rx_macros.h"

rxsw_render3d_t swRender3D;

BACKEND_EXPORT void Soft_Render3D_SyncCvars( ) { }

BACKEND_EXPORT void Soft_Render3D_ArenasInit( ) { Soft_Render3D_SurfaceArena_InitInvzBuffer( ); }

BACKEND_EXPORT void Soft_Render3D_ArenasShutdown( ) { Soft_Render3D_SurfaceArena_Shutdown( ); }

BACKEND_EXPORT void Soft_Render3D_RenderFlush( ) { }
BACKEND_EXPORT void Soft_Render3D_BaseInit( ) { }
