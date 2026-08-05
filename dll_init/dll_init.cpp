
#include "xash3d_types.h"
#include "ref_api.h"

#include "stubs.h"

#include "engine_stuff.h"

#include "rx_base.h"
#include "rx_backend.h"
#include "rx_video.h"
#include "rx_draw2d.h"
#include "rx_3d.h"
#include "rx_3d_entity.h"
#include "rx_triapi.h"
#include "rx_renderapi.h"
#include "rx_studioapi.h"
#include "rx_stats.h"
#include "rx_image_manager.h"

ref_api_t *engineFuncs;
ref_globals_t *refGlobals;
ref_client_t *refClient;
ref_host_t *refHost;

static const ref_interface_t exportedRefAPI = {
  &BaseInit_,              // R_Init
  &BaseShutdown_,          // R_Shutdown
  &GetRendererConfigName_, // R_GetConfigName
  &SetDisplayTransform_,   // R_SetDisplayTransform

  GL_SetupAttributes, // GL_SetupAttributes
  GL_InitExtensions,  // GL_InitExtensions
  GL_ClearExtensions, // GL_ClearExtensions

  &OnGammaChange_,             // R_GammaChanged
  &StartFrame_,                // R_BeginFrame
  &Render3D_Render3DSCENE_,    // R_RenderScene
  &EndFrame_,                  // R_EndFrame
  &Render3D_Entity_PushScene_, // R_PushScene
  &Render3D_Entity_PopScene_,  // R_PopScene
  GL_BackendStartFrame,        // GL_BackendStartFrame
  GL_BackendEndFrame,          // GL_BackendEndFrame

  &ClearScreen_,          // R_ClearScreen
  R_AllowFog,             // R_AllowFog, stub
  &TriAPI_SetRenderMode_, // GL_SetRenderMode

  &Render3D_Entity_AddEntity_,         // R_AddEntity
  &Render3D_Entity_ProcessEntityData_, // R_ProcessEntData

  R_ShowTextures, // R_ShowTextures, stub

  &ImageMGR_GetBaseImageData_, // R_GetTextureOriginalBuffer
  &ImageMGR_UpdateImage_,      // GL_LoadTextureFromBuffer
  GL_ProcessTexture,           // GL_ProcessTexture
  &SetupSkyboxImages_,         // R_SetupSky

  &Video_Set2DMode_,                // R_Set2DMode
  &Draw2D_DrawStretchImageWrapper_, // R_DrawStretchPic
  &Draw2D_FillRect_,                // FillRGBA
  WorldToScreen,                    // WorldToScreen

  VID_ScreenShot,  // VID_ScreenShot
  VID_CubemapShot, // VID_CubemapShot, stub

  R_LightPoint, // R_LightPoint

  R_DecalShoot,      // R_DecalShoot
  R_DecalRemoveAll,  // R_DecalRemoveAll
  R_CreateDecalList, // R_CreateDecalList
  R_ClearAllDecals,  // R_ClearAllDecals

  R_StudioEstimateFrame,        // R_StudioEstimateFrame
  R_StudioLerpMovement,         // R_StudioLerpMovement
  &StudioAPI_FillAPI_,          // R_StudioFillAPI
  &StudioAPI_SetDrawInterface_, // R_StudioSetDrawInterface

  &SetSkyCloudImages_, // R_SetSkyCloudsTextures
  GL_SubdivideSurface, // GL_SubdivideSurface, stub
  CL_RunLightStyles,   // CL_RunLightStyles, stub

  &ProcessModelRenderData_,    // Mod_ProcessRenderData
  &StudioAPI_LoadModelImages_, // Mod_StudioLoadTextures

  CL_DrawParticles, // CL_DrawParticles
  CL_DrawTracers,   // CL_DrawTracers
  CL_DrawBeams,     // CL_DrawBeams

  &GetRefParam_, // RefGetParm

  GetDetailScaleForTexture, // GetDetailScaleForTexture
  nullptr,                  // SetDetailScaleForTexture

  GL_CreateTexture,            // GL_CreateTexture
  &ImageMGR_FindImage_,        // GL_FindTexture
  &ImageMGR_GetImageName_,     // GL_TextureName
  &ImageMGR_GetBaseImageData_, // GL_TextureData
  &ImageMGR_LoadImage_,        // GL_LoadTexture
  &ImageMGR_FreeImageWrapper_, // GL_FreeTexture
  R_OverrideTextureSourceSize, // R_OverrideTextureSourceSize

  &ImageMGR_UpdateTexture_, // GL_UpdateTexture

  &ImageMGR_BindImage_, // GL_Bind

  &Render3DFrame_,       // GL_RenderFrame
  GL_OrthoBounds,        // GL_OrthoBounds, stub
  &RSpeeds_Message_,     // R_SpeedsMessage
  Mod_GetCurrentVis,     // Mod_GetCurrentVis, stub
  &NewMap_,              // R_NewMap
  &Render3D_ClearScene_, // R_ClearScene

  &TriAPI_SetRenderMode_,          // TriRenderMode
  &TriAPI_Immediate_Begin_,        // Begin
  &TriAPI_Immediate_End_,          // End
  &TriAPI_Immediate_Color4f_,      // Color4f
  &TriAPI_Immediate_Color4ub_,     // Color4ub
  &TriAPI_Immediate_AddVertex3fv_, // Vertex3fv
  &TriAPI_Immediate_AddVertex3f_,  // Vertex3f
  &TriAPI_CullFace_,               // CullFace

  &RenderAPI_FillAPI_, // R_FillRenderAPI
  &TriAPI_FillAPI_,    // R_FillTriAPI

  nullptr, // VGUI_SetupDrawing, stub
};

extern "C" EXPORT int GetRefAPI( int version, ref_interface_t *ref_funcs, ref_api_t *r_engfuncs, ref_globals_t *r_globals );
extern "C" EXPORT int GetRefAPI( int version, ref_interface_t *ref_funcs, ref_api_t *r_engfuncs, ref_globals_t *r_globals ) {
  // save ptrs
  engineFuncs = r_engfuncs;
  refGlobals = r_globals;
  refClient = ( ref_client_t * )( engineFuncs->EngineGetParm( PARM_GET_CLIENT_PTR, 0 ) );
  refHost = ( ref_host_t * )( engineFuncs->EngineGetParm( PARM_GET_HOST_PTR, 0 ) );

  // save engine functions ptr
  *ref_funcs = exportedRefAPI;

  return REF_API_VERSION;
}
