
#include "engine_stuff.h"

#include "rx_base.h"
#include "rx_cvars.h"
#include "rx_3d.h"

#include "rxgl_stuff.h"
#include "rxgl_base.h"
#include "rxgl_3d.h"
#include "rxgl_3d_surface.h"
#include "rx_stats.h"

#include "rx_macros.h"

rxgl_render3d_t glRender3D;

BACKEND_EXPORT void GL_Render3D_SyncCvars( ) {
}

BACKEND_EXPORT void GL_Render3D_ArenasInit( ) {
  GL_Render3D_Surface_ArenaInit( );

  renderSurfaceCount = 0;
  renderSurfaceVertexCount = 0;
  surfaceFlushCount = 0;
  surfaceFlushTime = 0.f;
}

BACKEND_EXPORT void GL_Render3D_ArenasShutdown( ) {
  GL_Render3D_Surface_ArenaShutdown( );
}

BACKEND_EXPORT void GL_Render3D_RenderFlush( ) {
  // flush the remaining surfaces if surface count > 0
  GL_Render3D_Surface_RenderAllSurfaces( );

#ifdef R_SPEEDS
  if ( cvarMirrors.r_speeds )
    R_SPEEDS_LOG( "%i surfs, %i verts, %i flushes, %f ms", renderSurfaceCount, renderSurfaceVertexCount, surfaceFlushCount, surfaceFlushTime );
#endif
}

bool GL_Render3D_BaseInit( ) {
  if ( !GL_Render3D_Surface_DrawSurfaceProgramInit( ) ) return false;

  glGenBuffers( 1, &glRender3D.render3DVBO );

  return true;
}
