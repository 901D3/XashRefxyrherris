
#pragma once

#include "com_model.h"
#include "cl_entity.h"

#include "rx_image.h"
#include "rx_blend.h"

#define RXSURF_WORLDMODEL           ( 1 << 0 ) // came from world model, excluding entity which are submodel of world model
#define RXSURF_ENTITY_SUBWORLDMODEL ( 1 << 1 )
#define RXSURF_ENTITY_STUDIO        ( 1 << 2 ) // came from studio entity
#define RXSURF_SKY                  ( 1 << 3 )
#define RXSURF_CONVEYOR             ( 1 << 4 ) // a conveyor surface
#define RXSURF_TURBULENCE           ( 1 << 5 )
#define RXSURF_UVCACHED             ( 1 << 6 ) // base texture uvs are cached, otherwise generate uv per vertex
#define RXSURF_TEXTURE_CHANGED      ( 1 << 7 )
#define RXSURF_MODELTRANSFORM       ( 1 << 8 ) // use model transform matrix to transform vectors

enum {
  UVCACHE_TEXTURE_U = 0,
  UVCACHE_TEXTURE_V,
  UVCACHE_LIGHTMAP_U,
  UVCACHE_LIGHTMAP_V,

  UVCACHE_ATTRCOUNT,
};

typedef struct rx_surface_s {
  msurface_t* mSurface;

  // if a surface has animation, we can grab a different image and put it here,
  // otherwise, use the initial texture stuff
  rx_image_t *baseTex, *fullbrightTex, *detailTex, *lightmapTex;

  int blendMode = BLEND_NORMAL;

  int alphaTestThresh = 0;
  int alphaVal;

  bool translucent;
  bool fullbright;

  bool isFromEntity;

  // RXSURF_*
  unsigned int flags;

  unsigned int reserved[ 2 ];

} rx_surfaceargs_t;

typedef struct {
  // for calculating uv (backend may differ)
  float** origVerts;
  // transformed vertices, should not be used for calculate uv
  float* transfVerts;
  int vertexCount;

} rx_surfacearena_t;

extern rx_surfacearena_t surfaceArena;

extern void Render3D_Surface_AddSurface( rx_surfaceargs_t* rxSurface );
extern void Render3D_SurfaceArena_Shutdown( );
