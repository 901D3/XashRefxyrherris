
#include <stdint.h>

#include "const.h"
#include "render_api.h"

#include "engine_stuff.h"

#include "rx_base.h"
#include "rx_renderapi.h"
#include "rx_image_manager.h"
#include "rx_3d.h"

#include "rx_macros.h"

#include "xyrherris-mathlib.h"

/*
## RenderAPI_GetExtraParmsForImage
*/
RENDERAPI_EXPORT void RenderAPI_GetExtraParmsForImage( int imageIdx, uint8_t *r, uint8_t *g, uint8_t *b, uint8_t *density ) {
  unsigned int fogColorRGBA = imageMGR.imageList[ imageIdx ].fogColorRGBA;

  if ( r )
    *r = M_getr( fogColorRGBA );

  if ( g )
    *g = M_getg( fogColorRGBA );

  if ( b )
    *b = M_getb( fogColorRGBA );

  if ( density )
    *density = M_geta( fogColorRGBA );
}

RENDERAPI_EXPORT void *RenderAPI_GetProcessAddr( const char *name ) {
  return engineFuncs->GL_GetProcAddress( name );
}

RENDERAPI_EXPORT float RenderAPI_GetFrameTime( ) {
  return refctx.frameTime;
}

RENDERAPI_EXPORT void RenderAPI_GetCurrentEntity( cl_entity_s *clEntity ) {
  // ref3D.currentEntity = clEntity;
}

RENDERAPI_EXPORT void RenderAPI_SetCurrentModel( model_t *model ) {
  // ref3D.currentModel = model;
}

RENDERAPI_EXPORT int GL_LoadTextureArray( const char **names, int flags ) {
  return 0;
}

RENDERAPI_EXPORT int GL_CreateTextureArray( const char *name, int width, int height, int depth, const void *buffer, texFlags_t flags ) {
  return 0;
}

RENDERAPI_EXPORT void DrawSingleDecal( decal_t *pDecal, msurface_t *fa ) {
}

RENDERAPI_EXPORT float *R_DecalSetupVerts( decal_t *pDecal, msurface_t *surf, int texture, int *outCount ) {
  return nullptr;
}

void R_EntityRemoveDecals( model_t *mod ) {
}

void GL_SelectTexture( int texture ) {
}

void GL_LoadTextureMatrix( const float *glmatrix ) {
}

void GL_TexMatrixIdentity( ) {
}

void GL_CleanUpTextureUnits( int last ) {
}

void GL_TexGen( unsigned int coord, unsigned int mode ) {
}

void GL_TextureTarget( uint target ) {
}

void GL_TexCoordArrayMode( uint mode ) {
}

void GL_UpdateTexSize( int texnum, int width, int height, int depth ) {
}

void GL_DrawParticles( const ref_viewpass_t *rvp, qboolean trans_pass, float frametime ) {
}

colorVec LightVec( const vec3_t start, const vec3_t end, vec3_t lspot, vec3_t lvec ) {
  static colorVec a = { 0, 0, 0, 0 };
  return a;
}

mstudiotexture_t *StudioGetTexture( cl_entity_t *e ) {
  return nullptr;
}

GAME_EXPORT void RenderAPI_FillAPI_( render_api_s *api ) {
  api->GetExtraParmsForTexture = RenderAPI_GetExtraParmsForImage;
  api->GetFrameTime = RenderAPI_GetFrameTime;
  api->R_SetCurrentEntity = RenderAPI_GetCurrentEntity;
  api->R_SetCurrentModel = RenderAPI_SetCurrentModel;
  api->GL_CreateTexture = ImageMGR_CreateImageWithBuffer_;
  api->GL_LoadTextureArray = GL_LoadTextureArray;
  api->GL_CreateTextureArray = GL_CreateTextureArray;
  api->DrawSingleDecal = DrawSingleDecal;
  api->R_DecalSetupVerts = R_DecalSetupVerts;
  api->R_EntityRemoveDecals = R_EntityRemoveDecals;
  api->GL_SelectTexture = GL_SelectTexture;
  api->GL_LoadTextureMatrix = GL_LoadTextureMatrix;
  api->GL_TexMatrixIdentity = GL_TexMatrixIdentity;
  api->GL_CleanUpTextureUnits = GL_CleanUpTextureUnits;
  api->GL_TexGen = GL_TexGen;
  api->GL_TextureTarget = GL_TextureTarget;
  api->GL_TexCoordArrayMode = GL_TexCoordArrayMode;
  api->GL_UpdateTexSize = GL_UpdateTexSize;
  api->GL_DrawParticles = GL_DrawParticles;
  api->LightVec = LightVec;
  api->StudioGetTexture = StudioGetTexture;
  api->GL_GetProcAddress = RenderAPI_GetProcessAddr;
}
