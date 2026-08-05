
#pragma once

#include "com_model.h"

#include "rx_3d_surface.h"
#include "rx_image.h"

enum {
  VERTEXPOOL_VX = 0,
  VERTEXPOOL_VY,
  VERTEXPOOL_VZ,

  VERTEXPOOL_TEXTURE_U,
  VERTEXPOOL_TEXTURE_V,

  VERTEXPOOL_LIGHTMAP_U,
  VERTEXPOOL_LIGHTMAP_V,

  PAD,

  VERTEXPOOL_ATTRCOUNT,
};

typedef struct {
  GLuint programId;

  struct {
    GLint position;
    GLint textureCoord;
  } attr;

  struct {
    GLint textureUnit;
    GLint mvp;
    GLint alphaTest;
    GLint alpha;
    GLint blendMode;
  } unif;

} rxgl_prog_drawsurface_t;

typedef struct {
  // we use first index to index to vertexPool
  // vertexCount is the number of vertices right after first index
  //
  // vertexPool:
  // v0 - v1 - v2 - v3 - v4
  //
  // first vertex = 1, the first vertex for the surface is v1
  // vertexCount = 4, last vertex for the surface is v4
  //
  // vertices that belongs to the surface:
  // v1 - v2 - v3 - v4

  rx_image_t *baseTex, *fullbrightTex;

  int firstVertexIdx;
  int vertexCount;

  float alphaTestThresh;
  float alphaVal;

  int blendMode;

} rxgl_surfdrawcmds_t;

typedef struct {
  rxgl_surfdrawcmds_t* surfaceList;
  int surfaceCount;
  int maxSurfaceCount;

  // vertex pool holds polygon vertex and texel pos
  // vx, vy, vz, tx, ty
  float* vertexPool;
  int vertexCount;
  int maxVertexCount;

} rxgl_surfacearena_t;

#ifdef R_SPEEDS
extern int renderSurfaceCount;
extern int renderSurfaceVertexCount;
extern int surfaceFlushCount;
extern float surfaceFlushTime;
#endif

extern void GL_Render3D_Surface_RenderAllSurfaces( );

extern void GL_Render3D_Surface_ProcessSurfaceGeneric( rx_surfaceargs_t* surfaceArgs, float* transfVerts, float** origVerts, int vertexCount );

extern void GL_Render3D_Surface_ArenaShutdown( );
extern void GL_Render3D_Surface_ArenaInit( );
extern bool GL_Render3D_Surface_DrawSurfaceProgramInit( );
