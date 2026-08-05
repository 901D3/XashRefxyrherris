
#include <float.h>

#include "xash3d_mathlib.h"

#include "rx_backend.h"
#include "rx_base.h"
#include "rx_video.h"
#include "rx_image.h"
#include "rx_image_manager.h"
#include "rx_3d.h"
#include "rx_3d_surface.h"

#include "rx_cvars.h"

#include "rx_logsutils.h"

#include "rx_memmgr.h"

#include "rx_macros.h"

#include "xyrherris-mathlib.h"

rx_surfacearena_t surfaceArena;

static inline void Render3D_SurfaceArena_BufferCheck( int vertexCount ) {
  if ( vertexCount > surfaceArena.vertexCount ) {
    surfaceArena.vertexCount = vertexCount;

    if ( surfaceArena.transfVerts ) ENGINE_FREE( surfaceArena.transfVerts );
    if ( surfaceArena.origVerts ) ENGINE_FREE( surfaceArena.origVerts );

    surfaceArena.transfVerts = ( float* )( ENGINE_ALLOC( refctx.memPool, ( vertexCount * 3 ) * SIZEOF_FLOAT ) );
    surfaceArena.origVerts = ( float** )( ENGINE_ALLOC( refctx.memPool, vertexCount * SIZEOF_INTPTR32 ) );
  }
}

static inline texture_t* Render3D_Surface_AnimateTexture( msurface_t* s ) {
  texture_t* base = s->texinfo->texture;
  int reletive;

  if ( render3D.currEntity && render3D.currEntity->curstate.frame ) {
    if ( base->alternate_anims )
      base = base->alternate_anims;
  }

  if ( !base->anim_total )
    return base;

  if ( base->name[ 0 ] == '-' ) {
    int tx = M_fastfloor( ( s->texturemins[ 0 ] + ( base->width << 16 ) ) / base->width ) % MOD_FRAMES,
        ty = M_fastfloor( ( s->texturemins[ 1 ] + ( base->height << 16 ) ) / base->height ) % MOD_FRAMES;

    reletive = refctx.randomTable[ ty * MOD_FRAMES + tx ] % base->anim_total;
  }
  else {
    int speed = 20;

    // Quake1 textures uses 10 frames per second
    if ( imageMGR.imageList[ base->gl_texturenum ].textureFlags & TF_QUAKEPAL )
      speed = 10;

    reletive = ( int )( refClient->time * speed ) % base->anim_total;
  }

  int i = 0;

  while ( base->anim_min > reletive || base->anim_max <= reletive ) {
    base = base->anim_next;

    if ( !base || ++i > MOD_FRAMES )
      return s->texinfo->texture;
  }

  return base;
}

void Render3D_Surface_AddSurface( rx_surfaceargs_t* surfaceArgs ) {
  model_t* model = render3D.currModel;
  medge16_t* modelEdges = model->edges16;

  msurface_t* mSurface = surfaceArgs->mSurface;

  texture_t* newTexture = Render3D_Surface_AnimateTexture( mSurface );

  surfaceArgs->baseTex = &imageMGR.imageList[ newTexture->gl_texturenum ];
  if ( mSurface->texinfo->texture->fb_texturenum ) surfaceArgs->fullbrightTex = &imageMGR.imageList[ newTexture->fb_texturenum ];
  if ( mSurface->texinfo->texture->dt_texturenum ) surfaceArgs->detailTex = &imageMGR.imageList[ newTexture->dt_texturenum ];

  if ( mSurface->flags & SURF_DRAWSKY ) surfaceArgs->flags |= RXSURF_SKY;
  if ( mSurface->flags & ( SURF_DRAWTURB_QUADS | SURF_DRAWTURB ) ) surfaceArgs->flags |= RXSURF_TURBULENCE;
  if ( mSurface->flags & SURF_CONVEYOR ) surfaceArgs->flags |= RXSURF_CONVEYOR;

  int vertexCount = mSurface->numedges;
  surfaceArgs->alphaVal = render3D.entityAlpha * 255.f;

  Render3D_SurfaceArena_BufferCheck( vertexCount );

  int firstEdgeIdx = mSurface->firstedge;
  int lastEdgeIdx = firstEdgeIdx + vertexCount;

  // clockwise winding order

  for ( int i = firstEdgeIdx, j = 0; i < lastEdgeIdx; i++, j++ ) {
    // index the edge of the surface of the current model
    int surfaceEdgeIdx = model->surfedges[ i ];

    medge16_t* edge16 = &modelEdges[ M_abstrick( surfaceEdgeIdx ) ];
    int vertexIdx = edge16->v[ surfaceEdgeIdx <= 0 ];

    int j3 = j * 3;

    float* vertex = ( float* )( model->vertexes[ vertexIdx ].position );

    surfaceArena.origVerts[ j ] = vertex;

    Render3D_TransformPoint( vertex, &surfaceArena.transfVerts[ j3 ] );
  }

  Backend_Render3D_Surface_ProcessSurfaceGeneric( surfaceArgs, surfaceArena.transfVerts, surfaceArena.origVerts, vertexCount );
}

void Render3D_SurfaceArena_Shutdown( ) {
  surfaceArena.vertexCount = 0;

  if ( surfaceArena.transfVerts ) ENGINE_FREE( surfaceArena.transfVerts );
  if ( surfaceArena.origVerts ) ENGINE_FREE( surfaceArena.origVerts );

  surfaceArena.transfVerts = nullptr;
  surfaceArena.origVerts = nullptr;
}
