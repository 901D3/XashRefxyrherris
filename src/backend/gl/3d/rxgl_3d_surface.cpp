
#include <stdint.h>
#include <limits.h>

#include "com_model.h"

#include "rx_image_manager.h"
#include "rx_video.h"
#include "rx_blend.h"
#include "rx_3d.h"
#include "rx_3d_surface.h"

#include "rxgl_stuff.h"
#include "rxgl_base.h"
#include "rxgl_cvars.h"
#include "rxgl_3d.h"
#include "rxgl_3d_surface.h"
#include "rxgl_triapi.h"

#include "rx_memmgr.h"

#include "rx_macros.h"

#ifdef R_SPEEDS
  #include <chrono>
#endif

static rxgl_surfacearena_t glSurfaceArena;

static rxgl_prog_drawsurface_t drawSurfaceProg;

#ifdef R_SPEEDS
int renderSurfaceCount;
int renderSurfaceVertexCount;
int surfaceFlushCount;
float surfaceFlushTime;
#endif

void GL_Render3D_Surface_RenderAllSurfaces( ) {

#ifdef R_SPEEDS
  auto t0 = std::chrono::steady_clock::now( );
#endif

  rxgl_surfdrawcmds_t* surfaceList = glSurfaceArena.surfaceList;

  GL_UseProgram( drawSurfaceProg.programId );

  glUniform1i( drawSurfaceProg.unif.textureUnit, 0 );
  GL_SetState( GL_DEPTH_TEST, true );

  glDepthFunc( GL_LEQUAL );
  GL_BindBuffer( GL_ARRAY_BUFFER, glRender3D.render3DVBO );

  glBufferSubData(
    GL_ARRAY_BUFFER,
    0,
    glSurfaceArena.vertexCount * VERTEXPOOL_ATTRCOUNT * SIZEOF_FLOAT,
    ( void* )( glSurfaceArena.vertexPool ) );

  glActiveTexture( GL_TEXTURE0 );
  glUniformMatrix4fv( drawSurfaceProg.unif.mvp, 1, GL_FALSE, render3D.mvp );

  glEnableVertexAttribArray( drawSurfaceProg.attr.position );
  glVertexAttribPointer(
    drawSurfaceProg.attr.position,
    3,
    GL_FLOAT,
    GL_FALSE,
    VERTEXPOOL_ATTRCOUNT * SIZEOF_FLOAT,
    ( void* )( 0 ) );

  glEnableVertexAttribArray( drawSurfaceProg.attr.textureCoord );
  glVertexAttribPointer(
    drawSurfaceProg.attr.textureCoord,
    2,
    GL_FLOAT,
    GL_FALSE,
    VERTEXPOOL_ATTRCOUNT * SIZEOF_FLOAT,
    ( void* )( VERTEXPOOL_TEXTURE_U * SIZEOF_FLOAT ) );

  float lastAlphaTestThresh = -1.f;
  GLuint lastTexHanndle = -1;
  int lastBlendMode = -1;

  for ( int i = 0; i < glSurfaceArena.surfaceCount; i++ ) {
    rxgl_surfdrawcmds_t* glDrawCmds = &surfaceList[ i ];

    if ( lastBlendMode != glDrawCmds->blendMode ) {
      lastBlendMode = glDrawCmds->blendMode;
      GL_TriAPI_SetBlendMode( glDrawCmds->blendMode );
      glUniform1i( drawSurfaceProg.unif.blendMode, glDrawCmds->blendMode );
    }

    glUniform1f( drawSurfaceProg.unif.alpha, glDrawCmds->alphaVal );

    if ( lastAlphaTestThresh != glDrawCmds->alphaTestThresh ) {
      lastAlphaTestThresh = glDrawCmds->alphaTestThresh;
      glUniform1f( drawSurfaceProg.unif.alphaTest, glDrawCmds->alphaTestThresh );
    }

    if ( lastTexHanndle != glDrawCmds->baseTex->glTexHandle ) {
      lastTexHanndle = glDrawCmds->baseTex->glTexHandle;
      glBindTexture( GL_TEXTURE_2D, glDrawCmds->baseTex->glTexHandle );
    }

    glDrawArrays( GL_TRIANGLE_FAN, glDrawCmds->firstVertexIdx, glDrawCmds->vertexCount );
  }

#ifdef R_SPEEDS
  surfaceFlushCount++;
  surfaceFlushTime += std::chrono::duration<unsigned int, std::nano>( std::chrono::steady_clock::now( ) - t0 ).count( ) / 1000000.f;
#endif
}

BACKEND_EXPORT void GL_Render3D_Surface_ProcessSurfaceGeneric( rx_surfaceargs_t* surfaceArgs, float* transfVerts, float** origVerts, int vertexCount ) {
  if ( vertexCount < 3 || glSurfaceArena.maxVertexCount < vertexCount ) // vertex pool size too low
    return;

  if ( glSurfaceArena.surfaceCount >= glSurfaceArena.maxSurfaceCount ||
    glSurfaceArena.vertexCount + vertexCount > glSurfaceArena.maxVertexCount ) {

    GL_Render3D_Surface_RenderAllSurfaces( );
    GL_Render3D_Surface_ArenaInit( );
  }

  msurface_t* mSurface = surfaceArgs->mSurface;

  rx_image_t* image = surfaceArgs->baseTex;

  rxgl_surfdrawcmds_t* glDrawCmds = &glSurfaceArena.surfaceList[ glSurfaceArena.surfaceCount ];
  float* vertexPool = &glSurfaceArena.vertexPool[ glSurfaceArena.vertexCount * VERTEXPOOL_ATTRCOUNT ];

  glDrawCmds->baseTex = image;
  glDrawCmds->firstVertexIdx = glSurfaceArena.vertexCount;
  glDrawCmds->alphaTestThresh = ( float )( surfaceArgs->alphaTestThresh ) / 255.f;
  glDrawCmds->blendMode = surfaceArgs->blendMode;
  glDrawCmds->vertexCount = vertexCount;
  glDrawCmds->baseTex = surfaceArgs->baseTex;
  glDrawCmds->alphaVal = ( int )( surfaceArgs->alphaVal ) / 255.f;

  bool cachedUV = surfaceArgs->flags & RXSURF_UVCACHED;

  // save the vertices that belongs to the current surface to vertex pool
  for ( int i = 0; i < vertexCount; i++ ) {
    int poolidx = i * VERTEXPOOL_ATTRCOUNT;
    int i3 = i * 3;

    vertexPool[ poolidx + VERTEXPOOL_VX ] = transfVerts[ i3 ];
    vertexPool[ poolidx + VERTEXPOOL_VY ] = transfVerts[ i3 + 1 ];
    vertexPool[ poolidx + VERTEXPOOL_VZ ] = transfVerts[ i3 + 2 ];

    float texu, texv, lightu, lightv;

    texu = ( DotProduct( origVerts[ i ], mSurface->texinfo->vecs[ 0 ] ) + mSurface->texinfo->vecs[ 0 ][ 3 ] ) / image->width;
    texv = ( DotProduct( origVerts[ i ], mSurface->texinfo->vecs[ 1 ] ) + mSurface->texinfo->vecs[ 1 ][ 3 ] ) / image->height;

    // lightu = ( DotProduct( origVerts[ i ], mSurface->info->lmvecs[ 0 ] ) + mSurface->info->lmvecs[ 0 ][ 3 ] ) / image->width;
    // lightv = ( DotProduct( origVerts[ i ], mSurface->info->lmvecs[ 1 ] ) + mSurface->info->lmvecs[ 1 ][ 3 ] ) / image->height;

    vertexPool[ poolidx + VERTEXPOOL_TEXTURE_U ] = texu;
    vertexPool[ poolidx + VERTEXPOOL_TEXTURE_V ] = texv;

    // vertexPool[ poolidx + VERTEXPOOL_LIGHTMAP_U ] = lightu;
    // vertexPool[ poolidx + VERTEXPOOL_LIGHTMAP_V ] = lightv;
  }

  glSurfaceArena.vertexCount += vertexCount;
  glSurfaceArena.surfaceCount++;

#ifdef R_SPEEDS
  renderSurfaceCount++;
  renderSurfaceVertexCount += vertexCount;
#endif
}

void GL_Render3D_Surface_ArenaShutdown( ) {
  if ( glSurfaceArena.surfaceList ) ENGINE_FREE( glSurfaceArena.surfaceList );
  if ( glSurfaceArena.vertexPool ) ENGINE_FREE( glSurfaceArena.vertexPool );

  glSurfaceArena.surfaceList = nullptr;
  glSurfaceArena.vertexPool = nullptr;

  glSurfaceArena.surfaceCount = 0;
  glSurfaceArena.vertexCount = 0;
}

void GL_Render3D_Surface_ArenaInit( ) {
  if ( glSurfaceArena.maxSurfaceCount != glCvarMirrors.rxgl_maxsurf.value || !glSurfaceArena.surfaceList ) {
    glSurfaceArena.maxSurfaceCount = glCvarMirrors.rxgl_maxsurf.value;

    if ( glSurfaceArena.surfaceList ) ENGINE_FREE( glSurfaceArena.surfaceList );

    glSurfaceArena.surfaceList = reinterpret_cast<rxgl_surfdrawcmds_t*>(
      ENGINE_ALLOC( refctx.memPool, glSurfaceArena.maxSurfaceCount * sizeof( rxgl_surfdrawcmds_t ) ) );
  }

  int size = VERTEXPOOL_ATTRCOUNT * SIZEOF_FLOAT;

  if ( glSurfaceArena.maxVertexCount != glCvarMirrors.rxgl_maxvert.value || !glSurfaceArena.vertexPool ) {
    glSurfaceArena.maxVertexCount = glCvarMirrors.rxgl_maxvert.value;

    if ( glSurfaceArena.vertexPool ) ENGINE_FREE( glSurfaceArena.vertexPool );

    size *= glSurfaceArena.maxVertexCount;

    glSurfaceArena.vertexPool = reinterpret_cast<float*>( ENGINE_ALLOC( refctx.memPool, size ) );

    glBufferData( GL_ARRAY_BUFFER, size, nullptr, GL_STREAM_DRAW );
  }
  else {
    GL_BindBuffer( GL_ARRAY_BUFFER, glRender3D.render3DVBO );

    glBufferData( GL_ARRAY_BUFFER, size * glSurfaceArena.maxVertexCount, nullptr, GL_STREAM_DRAW );
  }

  glSurfaceArena.surfaceCount = 0;
  glSurfaceArena.vertexCount = 0;
}

bool GL_Render3D_Surface_DrawSurfaceProgramInit( ) {
  const char* drawSurfaceVert =
#include "rxgl_s_3d_drawsurface.vert.h"
    ;
  const char* drawSurfaceFrag =
#include "rxgl_s_3d_drawsurface.frag.h"
    ;

  drawSurfaceProg.programId = M_CreateShaderProg( "drawSurfaceProg", drawSurfaceVert, drawSurfaceFrag );

  if ( !drawSurfaceProg.programId ) return false;

  M_GLProgSetAttr( &drawSurfaceProg, position, "a_Position" );
  M_GLProgSetAttr( &drawSurfaceProg, textureCoord, "a_TextureCoord" );

  M_GLProgSetUnif( &drawSurfaceProg, textureUnit, "u_TextureUnit" );
  M_GLProgSetUnif( &drawSurfaceProg, mvp, "u_MVP" );
  M_GLProgSetUnif( &drawSurfaceProg, alphaTest, "u_AlphaTest" );
  M_GLProgSetUnif( &drawSurfaceProg, alpha, "u_Alpha" );

  return true;
}
