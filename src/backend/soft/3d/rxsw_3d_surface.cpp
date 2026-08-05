#include <float.h>
#include <limits.h>

#include "xash3d_mathlib.h"

#include "rx_base.h"
#include "rx_3d.h"
#include "rx_3d_surface.h"
#include "rx_video.h"
#include "rx_image.h"
#include "rx_image_manager.h"
#include "rx_blend.h"

#include "rxsw_3d_surface.h"
#include "rxsw_3d_edge.h"

#include "rx_logsutils.h"

#include "rx_memmgr.h"

#include "rx_macros.h"

#include "xyrherris-mathlib.h"

#ifdef R_SPEEDS
int surfaceCount;
#endif

// optimization
#define RASTFLAGS_WRAP_TEXTURE ( 1 << 0 )

typedef struct {
  rx_svertex_t* transfSVerts;
  rx_svertex_t* clippedTransfSVerts;
  int vertexCount;

  float* invzBuffer;
  int invzBufferWidth;
  int invzBufferHeight;
} rxsw_surfacearena_t;

static rxsw_surfacearena_t swSurfaceArena;

static inline void Soft_Render3D_SurfaceArena_ScratchBufferCheck( int vertexCount ) {
  if ( vertexCount > swSurfaceArena.vertexCount ) {
    swSurfaceArena.vertexCount = vertexCount;

    if ( swSurfaceArena.transfSVerts ) ENGINE_FREE( swSurfaceArena.transfSVerts );
    if ( swSurfaceArena.clippedTransfSVerts ) ENGINE_FREE( swSurfaceArena.clippedTransfSVerts );

    swSurfaceArena.transfSVerts = ( rx_svertex_t* )( ENGINE_ALLOC( refctx.memPool, swSurfaceArena.vertexCount * sizeof( rx_svertex_t ) ) );
    swSurfaceArena.clippedTransfSVerts = ( rx_svertex_t* )( ENGINE_ALLOC( refctx.memPool, ( swSurfaceArena.vertexCount * 2 ) * sizeof( rx_svertex_t ) ) );
  }
}

static inline void Soft_Render3D_Surface_RasterShared(
  rx_surfaceargs_t* surfaceArgs,
  int idx,
  float invz, float udz, float vdz,
  pixrgba8_t* texData, int texWidth, int texHeight,
  int blendMode,
  unsigned int rastflags ) {

  if ( invz < swSurfaceArena.invzBuffer[ idx ] ) return;

  float z = 1.f / invz;

  int u = M_fastfloor( udz * z );
  int v = M_fastfloor( vdz * z );

  if ( rastflags & RASTFLAGS_WRAP_TEXTURE ) {
    if ( ( unsigned int )( u ) >= ( unsigned int )( texWidth ) ) {
      u %= texWidth;
      if ( u < 0 ) u += texWidth;
    }

    if ( ( unsigned int )( v ) >= ( unsigned int )( texHeight ) ) {
      v %= texHeight;
      if ( v < 0 ) v += texHeight;
    }
  }

  unsigned int src = texData[ v * texWidth + u ];

  if ( blendMode == BLEND_NORMAL ) {
    swSurfaceArena.invzBuffer[ idx ] = invz;
    videoctx.frameBuffer[ idx ] = src;
  }
  else if ( blendMode == BLEND_LERP ) {
    unsigned int alpha = M_geta( src );
    alpha = M_8BitMul( alpha, surfaceArgs->alphaVal );

    if ( alpha > surfaceArgs->alphaTestThresh ) {
      swSurfaceArena.invzBuffer[ idx ] = invz;

      unsigned int dst = videoctx.frameBuffer[ idx ];

      unsigned int rb = dst & 0xFF00FF;
      unsigned int g = dst & 0xFF00;

      unsigned int srcRB = src & 0xFF00FF;
      unsigned int srcG = src & 0xFF00;

      unsigned int blendedRB = rb + ( ( alpha * ( srcRB - rb ) ) >> 8 );
      unsigned int blendedG = g + ( ( alpha * ( srcG - g ) ) >> 8 );

      videoctx.frameBuffer[ idx ] = ( blendedRB & 0xFF00FF ) | ( blendedG & 0xFF00 );
    }
  }
}

void addedge( rxsw_edge_t* edge, rxsw_edge_t** edgeout0, rxsw_edge_t** edgeout1 ) {
  if ( !( *edgeout0 ) )
    *edgeout0 = edge;
  else if ( !( *edgeout1 ) )
    *edgeout1 = edge;
}

void activeedge( rxsw_edge_t* edge, rxsw_edge_t** edgeout0, rxsw_edge_t** edgeout1, int y ) {
  if ( y >= edge->startY && y < edge->endY )
    addedge( edge, edgeout0, edgeout1 );
}

static inline void Soft_Render3D_Surface_RasterizeTriangleScanline(
  rx_surfaceargs_t* surfaceArgs,
  rxsw_edge_t* edge0, rxsw_edge_t* edge1, rxsw_edge_t* edge2,
  float invzStart, float invzStepX, float invzStepY,
  float udzStart, float udzStepX, float udzStepY,
  float vdzStart, float vdzStepX, float vdzStepY,
  float compenX, float compenY,
  float rastflags ) {

  rx_image_t* image = surfaceArgs->baseTex;
  unsigned int* texData = image->dataRGBA8;

  int texWidth = image->width,
      texHeight = image->height;

  int minx = Q_max( Q_min( Q_min( edge0->startXFP, edge1->startXFP ), edge2->startXFP ) >> Q16_SHIFT, 0 ),
      maxx = Q_min( Q_max( Q_max( edge0->startXFP, edge1->startXFP ), edge2->startXFP ) >> Q16_SHIFT, videoctx.width - 1 );

  int miny = Q_max( Q_min( Q_min( edge0->startY, edge1->startY ), edge2->startY ), 0 ),
      maxy = Q_min( Q_max( Q_max( edge0->startY, edge1->startY ), edge2->startY ), videoctx.height - 1 );

  {
    rxsw_edge_t* t;

    if ( edge0->startY > edge1->startY ) M_swap( edge0, edge1, t );
    if ( edge1->startY > edge2->startY ) M_swap( edge1, edge2, t );
    if ( edge0->startY > edge1->startY ) M_swap( edge0, edge1, t );
  }

  if ( minx > maxx || miny > maxy ) return;

  float dx = minx - compenX,
        dy = miny - compenY;

  float invzRow = invzStart + dx * invzStepX + dy * invzStepY;
  float udzRow = udzStart + dx * udzStepX + dy * udzStepY;
  float vdzRow = vdzStart + dx * vdzStepX + dy * vdzStepY;

  rxsw_edge_t *activeEdge0, *activeEdge1;

  for ( int y = miny; y <= maxy; y++,
            invzRow += invzStepY,
            udzRow += udzStepY,
            vdzRow += vdzStepY ) {

    activeEdge0 = NULL;
    activeEdge1 = NULL;

    activeedge( edge0, &activeEdge0, &activeEdge1, y );
    activeedge( edge1, &activeEdge0, &activeEdge1, y );
    activeedge( edge2, &activeEdge0, &activeEdge1, y );

    if ( !activeEdge0 || !activeEdge1 ) continue;

    int row = y * videoctx.width;

    int edgedx0 = y - activeEdge0->startY,
        edgedx1 = y - activeEdge1->startY;

    int spanMin = ( activeEdge0->startXFP + edgedx0 * activeEdge0->stepXFP ) >> Q16_SHIFT;
    int spanMax = ( activeEdge1->startXFP + edgedx1 * activeEdge1->stepXFP ) >> Q16_SHIFT;

    if ( spanMin > spanMax ) {
      int t;
      M_swap( spanMin, spanMax, t );
    }

    spanMin = M_bound( 0, spanMin, videoctx.width - 1 );
    spanMax = M_bound( 0, spanMax, videoctx.width - 1 );

    float invz = invzRow + ( spanMin - minx ) * invzStepX,
          udz = udzRow + ( spanMin - minx ) * udzStepX,
          vdz = vdzRow + ( spanMin - minx ) * vdzStepX;

    for ( int x = spanMin; x <= spanMax; x++,
              invz += invzStepX,
              udz += udzStepX,
              vdz += vdzStepX ) {

      int idx = row + x;

      // Soft_Render3D_Surface_RasterShared( surfaceArgs, idx, invz, udz, vdz, texData, texWidth, texHeight, surfaceArgs->blendMode, rastflags );
    }
  }
}

static inline float Soft_Render3D_Surface_CalcGradients(
  rx_svertex_t* sv0, rx_svertex_t* sv1, rx_svertex_t* sv2,
  float* invzStepX, float* invzStepY,
  float* udzStepX, float* udzStepY,
  float* vdzStepX, float* vdzStepY ) {

  float dx1, dy1, dx2, dy2;

  float anchorX = sv0->projx,
        anchorY = sv0->projy,
        anchorInvz = sv0->invz;

  float determin = M_CalcGradientSteps(
    anchorX, anchorY,
    sv1->projx, sv1->projy,
    sv2->projx, sv2->projy,
    anchorInvz, sv1->invz, sv2->invz,
    invzStepX,
    invzStepY,
    &dx1, &dy1, &dx2, &dy2 );

  M_CalcGradientStepsPrecomp(
    dx1, dy1, dx2, dy2, determin,
    sv0->u * sv0->invz,
    sv1->u * sv1->invz,
    sv2->u * sv2->invz,
    udzStepX, udzStepY );

  M_CalcGradientStepsPrecomp(
    dx1, dy1, dx2, dy2, determin,
    sv0->v * sv0->invz,
    sv1->v * sv1->invz,
    sv2->v * sv2->invz,
    vdzStepX, vdzStepY );

  return determin;
}

static inline void Soft_Render3D_Surface_RenderSurfaceScanline( rx_surfaceargs_t* surfaceArgs, rx_svertex_t* svertices, int vertexCount, unsigned int rastflags ) {
  rx_svertex_t *sv0 = &svertices[ 0 ], *sv1, *sv2;

  float invzStepX = 0.f, invzStepY = 0.f, udzStepX = 0.f, udzStepY = 0.f, vdzStepX = 0.f, vdzStepY = 0.f;

  rxsw_edge_t edge0, edge1, edge2;

  for ( int i = 1; i < vertexCount - 1; i++ ) {
    sv1 = &svertices[ i ];
    sv2 = &svertices[ i + 1 ];

    Soft_Render3D_Edge_EmitEdgesP(
      sv0->projx, sv0->projy,
      sv1->projx, sv1->projy,
      sv2->projx, sv2->projy,
      &edge0, &edge1, &edge2 );

    float determin =
      Soft_Render3D_Surface_CalcGradients(
        sv0, sv1, sv2,
        &invzStepX, &invzStepY,
        &udzStepX, &udzStepY,
        &vdzStepX, &vdzStepY );

    if ( fabsf( determin ) < EPS ) continue;

    Soft_Render3D_Surface_RasterizeTriangleScanline(
      surfaceArgs,
      &edge0, &edge1, &edge2,
      sv0->invz, invzStepX, invzStepY,
      sv0->u * sv0->invz, udzStepX, udzStepY,
      sv0->v * sv0->invz, vdzStepX, vdzStepY,
      sv0->projx, sv0->projy, rastflags );
  }
}

static inline void Soft_Render3D_Surface_RasterizeTriangleEdgeEq(
  rx_surfaceargs_t* surfaceArgs,
  float projx0, float projy0,
  float projx1, float projy1,
  float projx2, float projy2,
  float invzStart, float invzStepX, float invzStepY,
  float udzStart, float udzStepX, float udzStepY,
  float vdzStart, float vdzStepX, float vdzStepY,
  float compenX, float compenY,
  float determin,
  unsigned int rastflags ) {

  rx_image_t* image = surfaceArgs->baseTex;
  unsigned int* texData = image->dataRGBA8;

  int texWidth = image->width,
      texHeight = image->height;

  float edge0_a = projy0 - projy1, edge0_b = projx1 - projx0, edge0_c = projx0 * projy1 - projx1 * projy0,
        edge1_a = projy1 - projy2, edge1_b = projx2 - projx1, edge1_c = projx1 * projy2 - projx2 * projy1,
        edge2_a = projy2 - projy0, edge2_b = projx0 - projx2, edge2_c = projx2 * projy0 - projx0 * projy2;

  if ( determin < 0.f ) {
    edge0_a = -edge0_a, edge0_b = -edge0_b, edge0_c = -edge0_c,
    edge1_a = -edge1_a, edge1_b = -edge1_b, edge1_c = -edge1_c,
    edge2_a = -edge2_a, edge2_b = -edge2_b, edge2_c = -edge2_c;
  }

  int minx = Q_max( Q_min( Q_min( projx0, projx1 ), projx2 ), 0 ),
      maxx = Q_min( Q_max( Q_max( projx0, projx1 ), projx2 ), videoctx.width - 1 );

  int miny = Q_max( Q_min( Q_min( projy0, projy1 ), projy2 ), 0 ),
      maxy = Q_min( Q_max( Q_max( projy0, projy1 ), projy2 ), videoctx.height - 1 );

  if ( minx > maxx || miny > maxy ) return;

  float minxmid = minx + 0.5f;
  float minymid = miny + 0.5f;

  float edgeRow0 = edge0_c + minxmid * edge0_a + minymid * edge0_b,
        edgeRow1 = edge1_c + minxmid * edge1_a + minymid * edge1_b,
        edgeRow2 = edge2_c + minxmid * edge2_a + minymid * edge2_b;

  float dx = minxmid - compenX,
        dy = minymid - compenY;

  float invzRow = invzStart + dx * invzStepX + dy * invzStepY,
        udzRow = udzStart + dx * udzStepX + dy * udzStepY,
        vdzRow = vdzStart + dx * vdzStepX + dy * vdzStepY;

  for ( int y = miny; y <= maxy; y++,
            edgeRow0 += edge0_b,
            edgeRow1 += edge1_b,
            edgeRow2 += edge2_b,

            invzRow += invzStepY,
            udzRow += udzStepY,
            vdzRow += vdzStepY ) {

    int spanTest = minx;
    int spanMin, spanMax;

    float edge0 = edgeRow0,
          edge1 = edgeRow1,
          edge2 = edgeRow2;

    while ( spanTest <= maxx && ( edge0 < 0 || edge1 < 0 || edge2 < 0 ) ) {
      edge0 += edge0_a, edge1 += edge1_a, edge2 += edge2_a;
      spanTest++;
    }

    spanMin = spanTest;

    while ( spanTest <= maxx && ( edge0 >= 0 && edge1 >= 0 && edge2 >= 0 ) ) {
      edge0 += edge0_a, edge1 += edge1_a, edge2 += edge2_a;
      spanTest++;
    }

    spanMax = spanTest - 1;

    if ( spanMin > spanMax ) continue;
    float offset = spanMin - minx;

    float invz = invzRow + offset * invzStepX,
          udz = udzRow + offset * udzStepX,
          vdz = vdzRow + offset * vdzStepX;

    int row = y * videoctx.width;

    for ( int x = spanMin; x <= spanMax; x++,
              invz += invzStepX,
              udz += udzStepX,
              vdz += vdzStepX ) {

      int idx = row + x;

      Soft_Render3D_Surface_RasterShared( surfaceArgs, idx, invz, udz, vdz, texData, texWidth, texHeight, surfaceArgs->blendMode, rastflags );
    }
  }
}

static inline void Soft_Render3D_Surface_RenderSurfaceEdgeEq(
  rx_surfaceargs_t* surfaceArgs,
  rx_svertex_t* svertices,
  int vertexCount,
  unsigned int rastflags ) {

  rx_svertex_t *sv0 = &svertices[ 0 ],
               *sv1 = &svertices[ 1 ],
               *sv2 = &svertices[ 2 ];

  float dx1, dy1, dx2, dy2;

  float invzStart, invzStepX, invzStepY,
    udzStart, udzStepX, udzStepY,
    vdzStart, vdzStepX, vdzStepY;

  for ( int i = 1; i < vertexCount - 1; i++ ) {
    sv1 = &svertices[ i ];
    sv2 = &svertices[ i + 1 ];

    float determin = Soft_Render3D_Surface_CalcGradients(
      sv0, sv1, sv2,
      &invzStepX, &invzStepY,
      &udzStepX, &udzStepY,
      &vdzStepX, &vdzStepY );

    Soft_Render3D_Surface_RasterizeTriangleEdgeEq(
      surfaceArgs,
      sv0->projx, sv0->projy,
      sv1->projx, sv1->projy,
      sv2->projx, sv2->projy,
      sv0->invz, invzStepX, invzStepY,
      sv0->u * sv0->invz, udzStepX, udzStepY,
      sv0->v * sv0->invz, vdzStepX, vdzStepY,
      sv0->projx, sv0->projy,
      determin,
      rastflags );
  }
}

BACKEND_EXPORT void Soft_Render3D_Surface_ProcessSurfaceGeneric( rx_surfaceargs_t* surfaceArgs, float* transfSverts, float** origVerts, int vertexCount ) {
  if ( vertexCount < 3 ) return;

  msurface_t* mSurface = surfaceArgs->mSurface;
  rx_image_t* baseTex = surfaceArgs->baseTex;

  Soft_Render3D_SurfaceArena_ScratchBufferCheck( vertexCount );

  unsigned int rastflags = 0;

  float *uvec, *vvec;
  if ( mSurface->texinfo->flags & TEX_WORLD_LUXELS ) {
    uvec = mSurface->texinfo->vecs[ 0 ];
    vvec = mSurface->texinfo->vecs[ 1 ];
  }
  else {
    uvec = mSurface->info->lmvecs[ 0 ];
    vvec = mSurface->info->lmvecs[ 1 ];
  }

  int minu = 0, maxu = 0, minv = 0, maxv = 0;

  for ( int i = 0; i < vertexCount; i++ ) {
    int i3 = i * 3;

    rx_svertex_t* currSVertex = &swSurfaceArena.transfSVerts[ i ];

    memcpy( currSVertex->position, &transfSverts[ i3 ], 3 * SIZEOF_FLOAT );

    // correcting texture
    currSVertex->u = DotProduct( origVerts[ i ], uvec ) + uvec[ 3 ] - 1.f;
    currSVertex->v = DotProduct( origVerts[ i ], vvec ) + vvec[ 3 ] - 1.f;

    minu = Q_min( minu, currSVertex->u ),
    maxu = Q_max( maxu, currSVertex->u ),
    minv = Q_min( minv, currSVertex->v ),
    maxv = Q_max( maxv, currSVertex->v );
  }

  if ( minu < 0 || maxu >= baseTex->width || minv < 0 || maxv >= baseTex->height )
    rastflags |= RASTFLAGS_WRAP_TEXTURE;

  int newVertexCount = Render3DUtils_ClipSPolygon(
    swSurfaceArena.transfSVerts, vertexCount,
    swSurfaceArena.clippedTransfSVerts, vertexCount * 2,
    render3D.frustumPlanes, FRUSTUM_PLANE_COUNT );

  if ( newVertexCount < 3 ) return;

  rx_svertex_t* clippedTransfSVerts = swSurfaceArena.clippedTransfSVerts;

  for ( int i = 0; i < newVertexCount; i++ ) {
    rx_svertex_t* currSVertex = &clippedTransfSVerts[ i ];

    currSVertex->invz =
      Render3D_WorldToScreen( currSVertex->position, render3D.position, &currSVertex->projx, &currSVertex->projy );
  }

  // Soft_Render3D_Surface_RenderSurfaceScanline( surfaceArgs, clippedTransfSVerts, newVertexCount );

  Soft_Render3D_Surface_RenderSurfaceEdgeEq( surfaceArgs, clippedTransfSVerts, newVertexCount, rastflags );
}

void Soft_Render3D_SurfaceArena_Shutdown( ) {
  swSurfaceArena.vertexCount = 0;

  if ( swSurfaceArena.invzBuffer ) ENGINE_FREE( swSurfaceArena.invzBuffer );
  if ( swSurfaceArena.transfSVerts ) ENGINE_FREE( swSurfaceArena.transfSVerts );
  if ( swSurfaceArena.clippedTransfSVerts ) ENGINE_FREE( swSurfaceArena.clippedTransfSVerts );

  swSurfaceArena.invzBuffer = nullptr;
  swSurfaceArena.transfSVerts = nullptr;
  swSurfaceArena.clippedTransfSVerts = nullptr;
}

void Soft_Render3D_SurfaceArena_InitInvzBuffer( ) {
  if ( !swSurfaceArena.invzBuffer ||
    swSurfaceArena.invzBufferWidth != videoctx.width ||
    swSurfaceArena.invzBufferHeight != videoctx.height ) {

    swSurfaceArena.invzBufferWidth = videoctx.width;
    swSurfaceArena.invzBufferHeight = videoctx.height;

    if ( swSurfaceArena.invzBuffer ) ENGINE_FREE( swSurfaceArena.invzBuffer );

    swSurfaceArena.invzBuffer = ( float* )( ENGINE_ALLOC( refctx.memPool, swSurfaceArena.invzBufferWidth * swSurfaceArena.invzBufferHeight * SIZEOF_FLOAT ) );
  }

  memset( swSurfaceArena.invzBuffer, 0, swSurfaceArena.invzBufferWidth * swSurfaceArena.invzBufferHeight * SIZEOF_FLOAT );
}
