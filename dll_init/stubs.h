
#pragma once

#include "port.h"
#include "xash3d_types.h"
#include "cvardef.h"
#include "const.h"
#include "com_model.h"
#include "cl_entity.h"
#include "render_api.h"
#include "protocol.h"
#include "ref_api.h"
#include "enginefeatures.h"
#include "cvardef.h"

#include "beamdef.h"
#include "lightstyle.h"
#include "triangleapi.h"

// ref_interface_s

// const char R_GetInitError(void);
void R_Shutdown(void);
const char *R_GetConfigName(void); // returns config name without extension
qboolean R_SetDisplayTransform(ref_screen_rotation_t rotate, int x, int y, float scale_x, float scale_y);

// only called for GL contexts
void GL_SetupAttributes(int safegl);
void GL_InitExtensions(void);
void GL_ClearExtensions(void);

// scene rendering
void R_GammaChanged(qboolean do_reset_gamma);
void R_BeginFrame(qboolean clearScene);
void R_RenderScene(void);
void R_PushScene(void);
void R_PopScene(void);
void GL_BackendStartFrame(void);
void GL_BackendEndFrame(void);

void R_ClearScreen(void); // clears color buffer on GL
void R_AllowFog(qboolean allow);
void GL_SetRenderMode(int renderMode);

qboolean R_AddEntity(struct cl_entity_s *clent, int type);
void CL_AddCustomBeam(cl_entity_t *pEnvBeam);
void R_ProcessEntData(qboolean allocate, cl_entity_t *entities, unsigned int max_entities);
void R_Flush(unsigned int flush_flags);

// debug
void R_ShowTextures(void);

// texture management
const byte *R_GetTextureOriginalBuffer(unsigned int idx);
int GL_LoadTextureFromBuffer(const char *name, rgbdata_t *pic, texFlags_t flags, qboolean update);
void GL_ProcessTexture(int texnum, float gamma, int topColor, int bottomColor);
void R_SetupSky(int *skyboxTextures);

// 2D
void R_Set2DMode(qboolean enable);
void R_DrawStretchRaw(float x, float y, float w, float h, int cols, int rows, const byte *data, qboolean dirty);
void R_DrawStretchPic(float x, float y, float w, float h, float s1, float t1, float s2, float t2, int texnum);
void FillRGBA(int rendermode, float x, float y, float w, float h, byte r, byte g, byte b, byte a);
int WorldToScreen(const vec3_t world, vec3_t screen);

// screenshot, cubemapshot
qboolean VID_ScreenShot(const char *filename, int shot_type);
qboolean VID_CubemapShot(const char *base, uint size, const float *vieworg, qboolean skyshot);

// light
colorVec R_LightPoint(const float *p);

// decals
// Shoots a decal onto the surface of the BSP.  position is the center of the decal in world coords
void R_DecalShoot(int textureIndex, int entityIndex, int modelIndex, vec3_t pos, int flags, float scale);
void R_DecalRemoveAll(int texture);
int R_CreateDecalList(struct decallist_s *pList);
void R_ClearAllDecals(void);

// studio interface
float R_StudioEstimateFrame(cl_entity_t *e, mstudioseqdesc_t *pseqdesc, double time);
void R_StudioLerpMovement(cl_entity_t *e, double time, vec3_t origin, vec3_t angles);
void CL_InitStudioAPI(void);

// bmodel
void R_SetSkyCloudsTextures(int solidskyTexture, int alphaskyTexture);
void GL_SubdivideSurface(model_t *mod, msurface_t *fa);
void CL_RunLightStyles(lightstyle_t *ls);

// sprites
void R_GetSpriteParms(int *frameWidth, int *frameHeight, int *numFrames, int currentFrame, const model_t *pSprite);
int R_GetSpriteTexture(const model_t *m_pSpriteModel, int frame);

// model management
// flags ignored for everything except spritemodels
qboolean Mod_ProcessRenderData(model_t *mod, qboolean create, const byte *buffer, size_t buffersize);
void Mod_StudioLoadTextures(model_t *mod, void *data);

// efx implementation
void CL_DrawParticles(double frametime, particle_t *particles, float partsize);
void CL_DrawTracers(double frametime, particle_t *tracers);
void CL_DrawBeams(int fTrans, BEAM *beams);
qboolean R_BeamCull(const vec3_t start, const vec3_t end, qboolean pvsOnly);

// Xash3D Render Interface
// Get renderer info (doesn't changes engine state at all)
int RefGetParm(int parm, int arg); // generic
void GetDetailScaleForTexture(int texture, float *xScale, float *yScale);
void GetExtraParmsForTexture(int texture, byte *red, byte *green, byte *blue, byte *alpha);
float GetFrameTime(void);

// Set renderer info (tell engine about changes)
void R_SetCurrentEntity(struct cl_entity_s *ent); // tell engine about both currententity and currentmodel
void R_SetCurrentModel(struct model_s *mod);      // change currentmodel but leave currententity unchanged

// Texture tools
int GL_FindTexture(const char *name);
const char *GL_TextureName(unsigned int texnum);
const byte *GL_TextureData(unsigned int texnum); // may be NULL
int GL_LoadTexture(const char *name, const byte *buf, size_t size, int flags);
int GL_CreateTexture(const char *name, int width, int height, const void *buffer, texFlags_t flags);
void GL_FreeTexture(unsigned int texnum);
void R_OverrideTextureSourceSize(unsigned int texnum, unsigned int srcWidth, unsigned int srcHeight);

// glState related calls (must use this instead of normal gl-calls to prevent de-synchornize local states between engine and the client)
void GL_Bind(int tmu, unsigned int texnum);
void GL_SelectTexture(int tmu);
void GL_LoadTextureMatrix(const float *glmatrix);
void GL_TexMatrixIdentity(void);
void GL_CleanUpTextureUnits(int last); // pass 0 for clear all the texture units
void GL_TexGen(unsigned int coord, unsigned int mode);
void GL_TextureTarget(unsigned int target); // change texture unit mode without bind texture
void GL_TexCoordArrayMode(unsigned int texmode);
void GL_UpdateTexSize(int texnum, int width, int height, int depth); // recalc statistics
// void GL_Reserved0(void); // for potential interface expansion without broken compatibility
// void GL_Reserved1(void);

// Misc renderer functions
void GL_DrawParticles(const struct ref_viewpass_s *rvp, qboolean trans_pass, float frametime);
colorVec LightVec(const float *start, const float *end, float *lightspot, float *lightvec);
struct mstudiotex_s *StudioGetTexture(struct cl_entity_s *e);

// passed through R_RenderFrame (0 - use engine renderer, 1 - use custom client renderer)
void GL_RenderFrame(const struct ref_viewpass_s *rvp);
// setup map bounds for ortho-projection when we in dev_overview mode
void GL_OrthoBounds(const float *mins, const float *maxs);
// grab r_speeds message
qboolean R_SpeedsMessage(char *out, size_t size);
// get visdata for current frame from custom renderer
byte *Mod_GetCurrentVis(void);
// tell the renderer what new map is started
void R_NewMap(void);
// clear the render entities before each frame
void R_ClearScene(void);
// GL_GetProcAddress for client renderer
void *R_GetProcAddress(const char *name);

// TriAPI Interface
// NOTE: implementation isn't required to be compatible
void TriRenderMode(int mode);
void Begin(int primitiveCode);
void End(void);
void Color4ub(unsigned char r, unsigned char g, unsigned char b, unsigned char a); // real glColor4ub
void TexCoord2f(float u, float v);
void Vertex3fv(const float *worldPnt);
void Vertex3f(float x, float y, float z);
void Fog(float flFogColor[3], float flStart, float flEnd, int bOn); // Works just like GL_FOG, flFogColor is r/g/b.
void ScreenToWorld(const float *screen, float *world);
void GetMatrix(const int pname, float *matrix);
void FogParams(float flDensity, int iFogSkybox);
void CullFace(TRICULLSTYLE mode);

// vgui drawing implementation
void VGUI_SetupDrawing(qboolean rect);
void VGUI_UploadTextureBlock(int drawX, int drawY, const byte *rgba, int blockWidth, int blockHeight);
