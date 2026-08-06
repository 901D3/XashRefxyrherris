
#ifdef GL
  #pragma message( "GL is defined" )
#else
  #pragma message( "GL is NOT defined" )
#endif

// debug for Windows only
#ifdef _DEBUG
  #define _CRTDBG_MAP_ALLOC
  #include <stdlib.h>
  #include <crtdbg.h>
#endif

#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#include <chrono>

#include "xash3d_types.h"
#include "const.h"
#include "xash3d_mathlib.h"
#include "ref_api.h"
#include "render_api.h"
#include "ref_params.h"
#include "com_model.h"
#include "q_client.h"
#include "mod_local.h"

#include "engine_stuff.h"

#include "rx_defs.h"
#include "rx_backend.h"
#include "rx_base.h"
#include "rx_cvars.h"
#include "rx_video.h"
#include "rx_image_manager.h"
#include "rx_3d.h"
#include "rx_3d_entity.h"
#include "rx_3d_surface.h"
#include "rx_triapi.h"
#include "rx_stats.h"
#include "rx_memmgr.h"

#include "rx_logsutils.h"

#include "xyrherris-mathlib.h"

#include "rx_memmgr.h"

#include "rx_macros.h"

rx_refctx_t refctx;

static auto startTime = std::chrono::steady_clock::now( );

GAME_EXPORT const char* GetRendererConfigName_( ) { return Backend_GetRendererConfigName( ); }
GAME_EXPORT void* GetProcessAddr_( const char* name ) { return engineFuncs->GL_GetProcAddress( name ); }

GAME_EXPORT qboolean SetDisplayTransform_( ref_screen_rotation_t rotate, int x, int y, float scaleX, float scaleY ) {
  bool returnValue = true;

  if ( rotate > 1 ) {
    dlog( "only 0 - 1 (no rotation and counter clockwise) rotation supported" );
    returnValue = false;
  }
  else
    videoctx.rotation = static_cast<int>( rotate );

  if ( x || y ) {
    dlog( "position offset not supported" );
    returnValue = false;
  }

  if ( scaleX != 1.f || scaleY != 1.f ) {
    dlog( "dimension scale not supported" );
    returnValue = false;
  }

  return returnValue;
}

GAME_EXPORT void OnGammaChange_( qboolean resetGamma ) { }
GAME_EXPORT void ClearScreen_( ) { Backend_ClearScreen( ); }

GAME_EXPORT void StartFrame_( qboolean clearScreen ) {
  startTime = std::chrono::steady_clock::now( );

  // sync cvars and also call callbacks when a cvar value changed
  Cvars_SyncCvarMirrors( );

  Video_Set2DMode_( true );

  Backend_StartFrame( );

  engineFuncs->CL_ExtraUpdate( );
}

GAME_EXPORT void EndFrame_( ) {
  Video_Set2DMode_( false );

  videoctx.viewChanged = false;

  if ( videoctx.width != refGlobals->width || videoctx.height != refGlobals->height ) {

    videoctx.viewChanged = true;

    videoctx.width = refGlobals->width;
    videoctx.height = refGlobals->height;

    Backend_Video_OnViewChange( );

    return;
  }

  Backend_EndFrame( );

  refctx.frameTime = std::chrono::duration<unsigned int, std::nano>( std::chrono::steady_clock::now( ) - startTime ).count( ) / 1000000.f;

#ifdef R_SPEEDS
  if ( cvarMirrors.r_speeds )
    R_SPEEDS_LOG( "ref: %.4f/s, %.4fms", 1000.f / refctx.frameTime, refctx.frameTime );
#endif
}

GAME_EXPORT int GetRefParam_( int parm, int arg ) {
  if ( parm == PARM_TEX_WIDTH )
    return imageMGR.imageList[ arg ].width;

  else if ( parm == PARM_TEX_HEIGHT )
    return imageMGR.imageList[ arg ].height;

  else if ( parm == PARM_TEX_SRC_WIDTH )
    return imageMGR.imageList[ arg ].width;

  else if ( parm == PARM_TEX_SRC_HEIGHT )
    return imageMGR.imageList[ arg ].height;

  else if ( parm == PARM_TEX_GLFORMAT )
    return 0;

  else if ( parm == PARM_TEX_ENCODE )
    return 0;

  else if ( parm == PARM_TEX_MIPCOUNT )
    return 1;

  else if ( parm == PARM_TEX_DEPTH )
    return 0;

  else if ( parm == PARM_TEX_SKYBOX )
    return refctx.image.skyBoxImageIdx[ arg ];

  else if ( parm == PARM_TEX_SKYTEXNUM )
    return 0;

  else if ( parm == PARM_TEX_LIGHTMAP )
    return refctx.image.lightMapImageIdx[ M_bound( 0, arg, MAX_LIGHTMAPS ) ];

  else if ( parm == PARM_TEX_TARGET )
    return 0;

  else if ( parm == PARM_TEX_TEXNUM )
    return imageMGR.imageList[ arg ].imageIdx;

  else if ( parm == PARM_TEX_FLAGS )
    return imageMGR.imageList[ arg ].rgbDataFlags;

  else if ( parm == PARM_TEX_MEMORY )
    return 0;

  else if ( parm == PARM_ACTIVE_TMU )
    return 0;

  else if ( parm == PARM_LIGHTSTYLEVALUE )
    return 0;

  else if ( parm == PARM_MAX_IMAGE_UNITS )
    return 0;

  else if ( parm == PARM_REBUILD_GAMMA )
    return 0;

  else if ( parm == PARM_GL_CONTEXT_TYPE )
    return 0;

  else if ( parm == PARM_GLES_WRAPPER )
    return 0;

  else if ( parm == PARM_STENCIL_ACTIVE )
    return 0;

  else if ( parm == PARM_SKY_SPHERE )
    return 0;

  else if ( parm == PARM_TEX_FILTERING )
    return 0;

  return engineFuncs->EngineGetParm( parm, arg );
}

GAME_EXPORT void SetupSkyboxImages_( int* skyBoxImageIdx ) {
  if ( !skyBoxImageIdx ) return;

  for ( int i = 0; i < SKYBOX_SIDES; i++ )
    refctx.image.skyBoxImageIdx[ i ] = skyBoxImageIdx[ i ];
}

GAME_EXPORT void SetSkyCloudImages_( int solidSkyImageIdx, int alphaSkyImageIdx ) {
  refctx.image.solidSkyImageIdx = solidSkyImageIdx;
  refctx.image.alphaSkyImageIdx = alphaSkyImageIdx;
}

GAME_EXPORT qboolean ProcessModelRenderData_( model_t* model, qboolean create, const uint8_t* buffer, size_t bufferSize ) {
  render3D.loadedGameplay = !!create;

  // if we disconnect from game, unload stuff
  if ( !create ) {
    if ( engineFuncs->drawFuncs->Mod_ProcessUserData )
      engineFuncs->drawFuncs->Mod_ProcessUserData( model, false, buffer );

    render3D.currModel = model;

    imageMGR.imageLoadFlags &= ~imageMGR.imageLoadFlags;

    Render3D_Shutdown( );
    UnloadTexturesForModel( model );

    return true;
  }

  imageMGR.imageLoadFlags |= IMGLOADFL_GENERATE_MIPS;

  modtype_t modelType = model->type;
  const bool validModelType = modelType == mod_studio || modelType == mod_brush || modelType == mod_alias || modelType == mod_sprite;

  if ( !validModelType ) {
    M_HostError( "Unsupported model type %i", modelType );
    return false;
  }

  if ( engineFuncs->drawFuncs->Mod_ProcessUserData )
    engineFuncs->drawFuncs->Mod_ProcessUserData( model, true, buffer );

  return validModelType;
}

GAME_EXPORT void Render3DFrame_( const ref_viewpass_s* engineViewPass ) {
  if ( !render3D.processFrame || refGlobals->width != videoctx.width || refGlobals->height != videoctx.height )
    return;

  // sync params
  Render3D_SyncViewpassParams( engineViewPass );

  if ( engineFuncs->drawFuncs->GL_RenderFrame ) {
    render3D.customRendering = true;

    if ( engineFuncs->drawFuncs->GL_RenderFrame( engineViewPass ) ) {
      render3D.frameCount++;

      render3D.resetVis = true;

      return;
    }
  }

  render3D.customRendering = false;

  if ( VIEWPASS_DRAW_WORLD )
    ;

  render3D.frameCount++;
  Render3D_Render3DSCENE_( );
}

GAME_EXPORT void NewMap_( ) {
  if ( render3D.loadedGameplay ) Render3D_Shutdown( );

  model_t* worldBspModel = WORLD_MODEL;

  Render3D_Entity_ClearCurrentDrawList( );

  if ( worldBspModel->flags & MODEL_QBSP2 ) {
    M_HostError( "xashrefxyr can not load maps in BSP2 format" );
    return;
  }

  for ( int i = 0; i < worldBspModel->numleafs; i++ )
    worldBspModel->leafs[ i + 1 ].efrags = nullptr;

  render3D.mapSampleBits = -1;
  render3D.mapSampleSize = engineFuncs->Mod_SampleSizeForFace( &worldBspModel->surfaces[ 0 ] );

  for ( int i = 1; i < worldBspModel->numsurfaces; i++ ) {
    int sampleSize = engineFuncs->Mod_SampleSizeForFace( &worldBspModel->surfaces[ i ] );
    if ( sampleSize != render3D.mapSampleSize ) {
      render3D.mapSampleSize = -1;
      break;
    }
  }

  if ( render3D.mapSampleSize != -1 ) {
    render3D.mapSampleBits = 0;

    for ( int i = 1; i < render3D.mapSampleSize; i <<= 1, render3D.mapSampleBits++ );
  }

  dlog( "Map sample size %i, bits %i", render3D.mapSampleSize, render3D.mapSampleBits );

  Render3D_OnMapLoad( );
}

void UnloadTexturesForModel( model_t* model ) {
  const modtype_t modelType = model->type;

  if ( modelType == mod_studio || modelType == mod_alias || modelType == mod_sprite )
    return;

  else if ( modelType == mod_brush )
    ImageMGR_UnloadBrushImages( model );

  else
    M_HostError( "Unsupported model type %i", modelType );
}

static inline void InitRandomTable( ) {
  for ( int i = 0; i < MOD_FRAMES; i++ ) {
    int row = i * MOD_FRAMES;

    for ( int j = 0; j < MOD_FRAMES; j++ )
      refctx.randomTable[ row + j ] = engineFuncs->COM_RandomLong( 0, 0x7FFFFFFF );
  }
}

GAME_EXPORT void BaseShutdown_( ) {
  Backend_Shutdown( );

  Video_Shutdown( );
  ImageMGR_Shutdown( );
  TriAPI_ArenaShutdown( );

  engineFuncs->R_Free_Video( );

  ENGINE_FREE_POOL( &refctx.memPool );

#ifdef _DEBUG
  _CrtDumpMemoryLeaks( );
#endif
}

GAME_EXPORT qboolean BaseInit_( ) {

// debug for Windows only
#ifdef _DEBUG
  static bool initialized = false;

  if ( !initialized ) {
    initialized = true;

    _CrtSetDbgFlag(
      _CRTDBG_ALLOC_MEM_DF |
      _CRTDBG_LEAK_CHECK_DF |
      _CRTDBG_CHECK_ALWAYS_DF );

    _CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE );
    _CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDOUT );
  }
#endif

  refctx.memPool = ENGINE_ALLOC_POOL( "ref_xashrefxyr pool" );

  bool init = true;

  if ( !Backend_Init( ) ) init = false;
  if ( !Video_Init( ) ) init = false;
  if ( !ImageMGR_Init( ) ) init = false;

  if ( !init ) {
    BaseShutdown_( );
    return false;
  }

  if ( !TriAPI_Init( ) ) warnlog( "could not initialize TriAPI" );

  Cvars_CvarsInit( );

  refctx.backend = Backend_GetBackendEnumerator( );

  // refctx.params.world = reinterpret_cast<world_static_s *>( engineFuncs->EngineGetParm( PARM_GET_WORLD_PTR, 0 ) );
  // refctx.params.palette = reinterpret_cast<color24 *>( engineFuncs->EngineGetParm( PARM_GET_PALETTE_PTR, 0 ) );
  // refctx.params.viewEntity = reinterpret_cast<cl_entity_t *>( engineFuncs->EngineGetParm( PARM_GET_VIEWENT_PTR, 0 ) );
  // refctx.params.textureGamma = reinterpret_cast<uint8_t *>( engineFuncs->EngineGetParm( PARM_GET_TEXGAMMATABLE_PTR, 0 ) );
  // refctx.params.lightGamma = reinterpret_cast<unsigned int *>( engineFuncs->EngineGetParm( PARM_GET_LIGHTGAMMATABLE_PTR, 0 ) );
  // refctx.params.screenGamma = reinterpret_cast<unsigned int *>( engineFuncs->EngineGetParm( PARM_GET_SCREENGAMMATABLE_PTR, 0 ) );
  // refctx.params.linearGamma = reinterpret_cast<unsigned int *>( engineFuncs->EngineGetParm( PARM_GET_LINEARGAMMATABLE_PTR, 0 ) );
  // refctx.params.eLights = reinterpret_cast<dlight_t *>( engineFuncs->EngineGetParm( PARM_GET_ELIGHTS_PTR, 0 ) );

  InitRandomTable( );

  Render3D_InitScene( );

  render3D.processFrame = true;

  return true;
}
