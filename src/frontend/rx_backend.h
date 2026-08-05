
#pragma once

#ifdef GL
  #include "rxgl_base.h"
  #include "rxgl_cvars.h"
  #include "rxgl_video.h"
  #include "rxgl_image_manager.h"
  #include "rxgl_draw2d.h"
  #include "rxgl_3d.h"
  #include "rxgl_3d_surface.h"
  #include "rxgl_triapi.h"
  #include "rxgl_studioapi.h"

  #define GetBackend( functionName ) GL_##functionName
#else
  #include "rxsw_base.h"
  #include "rxsw_cvars.h"
  #include "rxsw_video.h"
  #include "rxsw_image_manager.h"
  #include "rxsw_3d.h"
  #include "rxsw_3d_surface.h"
  #include "rxsw_triapi.h"
  #include "rxsw_studioapi.h"

  #define GetBackend( functionName ) Soft_##functionName
#endif

#define Backend_GetBackendEnumerator  GetBackend( GetBackendEnumerator )
#define Backend_GetRendererConfigName GetBackend( GetRendererConfigName )
#define Backend_ClearScreen           GetBackend( ClearScreen )
#define Backend_StartFrame            GetBackend( StartFrame )
#define Backend_EndFrame              GetBackend( EndFrame )
#define Backend_InitAllArenas         GetBackend( InitAllArenas )
#define Backend_Shutdown              GetBackend( Shutdown )
#define Backend_Init                  GetBackend( Init )

#define Backend_Draw2D_FillRect         GetBackend( Draw2D_FillRect )
#define Backend_Draw2D_DrawStretchImage GetBackend( Draw2D_DrawStretchImage )

#define Backend_Video_Set2DMode    GetBackend( Video_Set2DMode )
#define Backend_Video_OnViewChange GetBackend( Video_OnViewChange )
#define Backend_Video_Shutdown     GetBackend( Video_Shutdown )
#define Backend_Video_Init         GetBackend( Video_Init )

#define Backend_TriAPI_SetBlendMode    GetBackend( TriAPI_SetBlendMode )
#define Backend_TriAPI_Immediate_Begin GetBackend( TriAPI_Immediate_Begin )
#define Backend_TriAPI_Immediate_End   GetBackend( TriAPI_Immediate_End )
#define Backend_TriAPI_AddVertex3f     GetBackend( TriAPI_AddVertex3f )
#define Backend_TriAPI_CullFace        GetBackend( TriAPI_CullFace )
#define Backend_TriAPI_Init            GetBackend( TriAPI_Init )

#define Backend_Cvars_SetCvar         GetBackend( Cvars_SetCvar )
#define Backend_Cvars_SyncCvarMirrors GetBackend( Cvars_SyncCvarMirrors )
#define Backend_Cvars_CvarsInit       GetBackend( Cvars_CvarsInit )

#define Backend_ImageMGR_LoadImage GetBackend( ImageMGR_LoadImage )
#define Backend_ImageMGR_FreeImage GetBackend( ImageMGR_FreeImage )
#define Backend_ImageMGR_Init      GetBackend( ImageMGR_Init )

#define Backend_Render3D_SyncCvars      GetBackend( Render3D_SyncCvars )
#define Backend_Render3D_ArenasInit     GetBackend( Render3D_ArenasInit )
#define Backend_Render3D_ArenasShutdown GetBackend( Render3D_ArenasShutdown )
#define Backend_Render3D_OnSyncViewpass GetBackend( Render3D_OnSyncViewpass )
#define Backend_Render3D_RenderFlush    GetBackend( Render3D_RenderFlush )

#define Backend_Render3D_Surface_ProcessSurfaceGeneric GetBackend( Render3D_Surface_ProcessSurfaceGeneric )

#define Backend_StudioAPI_RestoreRenderer GetBackend( StudioAPI_RestoreRenderer )
