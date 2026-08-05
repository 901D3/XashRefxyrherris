
#include "const.h"
#include "xash3d_types.h"

#include "engine_stuff.h"
#include "stubs.h"

#include "rx_base.h"
#include "rx_video.h"
#include "rx_image_manager.h"

// ref_interface_s

GAME_EXPORT void GL_SetupAttributes( int safegl ) { }
GAME_EXPORT void GL_InitExtensions( void ) { }
GAME_EXPORT void GL_ClearExtensions( void ) { }
GAME_EXPORT void GL_BackendStartFrame( void ) { }
GAME_EXPORT void GL_BackendEndFrame( void ) { }
GAME_EXPORT void R_ClearScreen( void ) { }
GAME_EXPORT void R_AllowFog( qboolean allow ) { }
GAME_EXPORT void CL_AddCustomBeam( cl_entity_t *pEnvBeam ) { }
GAME_EXPORT void R_Flush( unsigned int flush_flags ) { }
GAME_EXPORT void R_ShowTextures( void ) { }
GAME_EXPORT const byte *R_GetTextureOriginalBuffer( unsigned int idx ) { return NULL; }
GAME_EXPORT int GL_LoadTextureFromBuffer( const char *name, rgbdata_t *pic, texFlags_t flags, qboolean update ) { return 0; }
GAME_EXPORT void GL_ProcessTexture( int texnum, float gamma, int topColor, int bottomColor ) { }
GAME_EXPORT void R_SetupSky( int *skyboxTextures ) { }
GAME_EXPORT void R_DrawStretchRaw( float x, float y, float w, float h, int cols, int rows, const byte *data, qboolean dirty ) { }
GAME_EXPORT void R_DrawStretchPic( float x, float y, float w, float h, float s1, float t1, float s2, float t2, int texnum ) { }
GAME_EXPORT int WorldToScreen( const vec3_t world, vec3_t screen ) { return 0; }
GAME_EXPORT qboolean VID_ScreenShot( const char *filename, int shot_type ) { return 0; }
GAME_EXPORT qboolean VID_CubemapShot( const char *base, uint size, const float *vieworg, qboolean skyshot ) { return false; }
GAME_EXPORT colorVec R_LightPoint( const float *p ) {
  colorVec v = { 0 };
  return v;
}
GAME_EXPORT void R_DecalShoot( int textureIndex, int entityIndex, int modelIndex, vec3_t pos, int flags, float scale ) { }
GAME_EXPORT void R_DecalRemoveAll( int texture ) { }
GAME_EXPORT int R_CreateDecalList( struct decallist_s *pList ) { return 0; }
GAME_EXPORT void R_ClearAllDecals( void ) { }
GAME_EXPORT float R_StudioEstimateFrame( cl_entity_t *e, mstudioseqdesc_t *pseqdesc, double time ) { return 0.f; }
GAME_EXPORT void R_StudioLerpMovement( cl_entity_t *e, double time, vec3_t origin, vec3_t angles ) { }
GAME_EXPORT void R_SetSkyCloudsTextures( int solidskyTexture, int alphaskyTexture ) { }
GAME_EXPORT void GL_SubdivideSurface( model_t *mod, msurface_t *fa ) { }
GAME_EXPORT void CL_RunLightStyles( lightstyle_t *ls ) { }
GAME_EXPORT void R_GetSpriteParms( int *frameWidth, int *frameHeight, int *numFrames, int currentFrame, const model_t *pSprite ) { }
GAME_EXPORT int R_GetSpriteTexture( const model_t *m_pSpriteModel, int frame ) { return 0; }
GAME_EXPORT qboolean Mod_ProcessRenderData( model_t *mod, qboolean create, const byte *buffer, size_t buffersize ) { return 0; }
GAME_EXPORT void Mod_StudioLoadTextures( model_t *mod, void *data ) { }
GAME_EXPORT void CL_DrawParticles( double frametime, particle_t *particles, float partsize ) { }
GAME_EXPORT void CL_DrawTracers( double frametime, particle_t *tracers ) { }
GAME_EXPORT void CL_DrawBeams( int fTrans, BEAM *beams ) { }
GAME_EXPORT qboolean R_BeamCull( const vec3_t start, const vec3_t end, qboolean pvsOnly ) { return 0; }
GAME_EXPORT int RefGetParm( int parm, int arg ) { return 0; }
GAME_EXPORT void GetDetailScaleForTexture( int texture, float *xScale, float *yScale ) { }
GAME_EXPORT void GetExtraParmsForTexture( int texture, byte *red, byte *green, byte *blue, byte *alpha ) { }
GAME_EXPORT float GetFrameTime( void ) { return 0.f; }
GAME_EXPORT void R_SetCurrentEntity( struct cl_entity_s *ent ) { }
GAME_EXPORT void R_SetCurrentModel( struct model_s *mod ) { }
GAME_EXPORT int GL_FindTexture( const char *name ) { return 0; }
GAME_EXPORT int GL_CreateTexture( const char *name, int width, int height, const void *buffer, texFlags_t flags ) { return 0; }
GAME_EXPORT void GL_FreeTexture( unsigned int texnum ) { }
GAME_EXPORT void R_OverrideTextureSourceSize( unsigned int texnum, unsigned int srcWidth, unsigned int srcHeight ) { }
GAME_EXPORT void GL_RenderFrame( const struct ref_viewpass_s *rvp ) { }
GAME_EXPORT void GL_OrthoBounds( const float *mins, const float *maxs ) { }
GAME_EXPORT qboolean R_SpeedsMessage( char *out, size_t size ) { return false; }
GAME_EXPORT byte *Mod_GetCurrentVis( void ) { return NULL; }
GAME_EXPORT void R_NewMap( void ) { }
GAME_EXPORT void R_ClearScene( void ) { }
GAME_EXPORT void *R_GetProcAddress( const char *name ) { return engineFuncs->GL_GetProcAddress( name ); } // TriAPI Interface// NOTE: implementation isn't required to be compatibleGAME_EXPORT void TriRenderMode(int mode) {}GAME_EXPORT void Begin( int primitiveCode ) { }GAME_EXPORT void End( void ) {}GAME_EXPORT void Color4ub( unsigned char r, unsigned char g, unsigned char b, unsigned char a ) { }
GAME_EXPORT void TexCoord2f( float u, float v ) { }
GAME_EXPORT void Vertex3fv( const float *worldPnt ) { }
GAME_EXPORT void Vertex3f( float x, float y, float z ) { }
GAME_EXPORT void Fog( float flFogColor[ 3 ], float flStart, float flEnd, int bOn ) { }
GAME_EXPORT void ScreenToWorld( const float *screen, float *world ) { }
GAME_EXPORT void GetMatrix( const int pname, float *matrix ) { }
GAME_EXPORT void FogParams( float flDensity, int iFogSkybox ) { }
GAME_EXPORT void CullFace( TRICULLSTYLE mode ) { }
GAME_EXPORT void VGUI_SetupDrawing( qboolean rect ) { }
GAME_EXPORT void VGUI_UploadTextureBlock( int drawX, int drawY, const byte *rgba, int blockWidth, int blockHeight ) { }