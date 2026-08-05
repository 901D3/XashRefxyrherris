
#include <stdint.h>

#include "xash3d_types.h"

extern "C" {
#include "xash3d_mathlib.h"
}

#include "com_model.h"
#include "cl_entity.h"
#include "cvardef.h"
#include "const.h"
#include "r_studioint.h"
#include "studio.h"
#include "enginefeatures.h"

#include "engine_stuff.h"

#include "rx_backend.h"

#include "rx_base.h"
#include "rx_3d.h"
#include "rx_studioapi.h"
#include "rx_blend.h"

#include "xyrherris-mathlib.h"

#include "rx_macros.h"

r_studio_interface_t *engineStudioDraw;

rx_studioctx_t studioAPI;

// for engine getting bone transform arrays
// this is only for getter and nothing else
float ***boneTransf[ MAXSTUDIOBONES ];
float ***lightTransf[ MAXSTUDIOBONES ];

GAME_EXPORT void StudioAPI_LoadModelImages_( model_t *model, void *data ) {
  unsigned char *buffer = reinterpret_cast<unsigned char *>( data );

  studiohdr_t *modelExtraData = reinterpret_cast<studiohdr_t *>( buffer );
  if ( !modelExtraData ) return;

  if ( modelExtraData->textureindex > 0 ) {
    mstudiotexture_t *textureList = reinterpret_cast<mstudiotexture_t *>( buffer + modelExtraData->textureindex );

    for ( int i = 0; i < modelExtraData->numtextures; i++ )
      StudioAPI_LoadModelImage( model, modelExtraData, &textureList[ i ] );
  }
}

GAME_EXPORT void StudioAPI_SetDrawInterface_( r_studio_interface_s *pDraw ) { engineStudioDraw = pDraw; }

void StudioAPI_SetupTimings( ) {
  if ( render3D.viewpassFlags & RF_DRAW_WORLD ) {
    studioAPI.time = refClient->time;
    studioAPI.frameTime = refClient->time - refClient->oldtime;
  }
  else {
    studioAPI.time = refHost->realtime;
    studioAPI.frameTime = refHost->frametime;
  }
}

float StudioAPI_EstimateFrame( cl_entity_t *clEntity, mstudioseqdesc_t *sequenceDesc, double time ) {
  double dfdt = 0.0;
  double frame;

  if ( sequenceDesc->numframes <= 1 )
    frame = 0.0;
  else
    frame = ( clEntity->curstate.frame * ( sequenceDesc->numframes - 1 ) ) / 256.f;

  frame += dfdt;

  if ( sequenceDesc->flags & STUDIO_LOOPING ) {
    if ( sequenceDesc->numframes > 1 )
      frame -= M_floor( frame / ( sequenceDesc->numframes - 1 ) ) * ( sequenceDesc->numframes - 1 );
    if ( frame < 0.0 )
      frame += ( sequenceDesc->numframes - 1 );
  }
  else
    frame = M_bound( 0.0, frame, sequenceDesc->numframes - 1.001 );

  return frame;
}

void StudioAPI_DrawNormalMesh( short *triCommands, vec3_t *meshNormal, float s, float t ) {
}

void StudioAPI_SetColorBegin( short *triCommands, vec3_t *meshNormal ) {
  unsigned int color;
}

STUDIOAPI_EXPORT player_info_s *StudioAPI_GetPlayerInfo( int index ) { return engineFuncs->pfnPlayerInfo( ( render3D.viewpassFlags & RF_DRAW_WORLD ) ? 1 : index ); }
STUDIOAPI_EXPORT entity_state_s *StudioAPI_GetPlayerState( int index ) { return ( render3D.viewpassFlags & RF_DRAW_WORLD ) ? &render3D.currEntity->curstate : engineFuncs->pfnGetPlayerState( index ); }

STUDIOAPI_EXPORT void StudioAPI_GetTimes( int *frameCount, double *currentTime, double *previousTime ) {
  if ( frameCount ) *frameCount = render3D.frameCount;
  if ( currentTime ) *currentTime = refClient->time;
  if ( previousTime ) *previousTime = refClient->oldtime;
}

STUDIOAPI_EXPORT void StudioAPI_GetViewpassInfo( float origin[], float upView[], float rightView[], float forwardView[] ) {
  if ( origin ) VectorCopy( render3D.position, origin );
  if ( rightView ) VectorCopy( &render3D.angleVector[ ANGLEVEC_RIGHT_IDX ], rightView );
  if ( upView ) VectorCopy( &render3D.angleVector[ ANGLEVEC_UP_IDX ], upView );
  if ( forwardView ) VectorCopy( &render3D.angleVector[ ANGLEVEC_FORWARD_IDX ], forwardView );
}

STUDIOAPI_EXPORT void StudioAPI_GetModelCounters( int **s, int **a ) { *s = &studioAPI.frameCount, *a = 0; }

STUDIOAPI_EXPORT float ****StudioAPI_GetBoneTransform( ) { return reinterpret_cast<float ****>( boneTransf ); }
STUDIOAPI_EXPORT float ****StudioAPI_GetLightTransform( ) { return reinterpret_cast<float ****>( lightTransf ); }
STUDIOAPI_EXPORT float ***StudioAPI_GetRotationMatrix( ) { return reinterpret_cast<float ***>( studioAPI.rotationMatrix ); }

STUDIOAPI_EXPORT int StudioAPI_CheckBoundingBox( ) { return 0; }
STUDIOAPI_EXPORT void StudioDynamicLight( cl_entity_s *ent, alight_s *plight ) { }
STUDIOAPI_EXPORT void StudioEntityLight( alight_s *plight ) { }

STUDIOAPI_EXPORT void StudioAPI_SetupLighting( alight_s *light ) {
  if ( !studioAPI.modelHeader || !light ) return;

  float scale = 1.f;
  if ( render3D.currEntity ) scale = render3D.currEntity->curstate.scale;

  memcpy( &studioAPI.light, light, sizeof( alight_t ) );

  for ( int i = 0; i < studioAPI.modelHeader->numbones; i++ ) {
    Matrix3x4_VectorIRotate( studioAPI.boneTransf[ i ].lightTransf, light->plightvec, studioAPI.lightVector[ i ] );
    if ( scale > 1.f ) VectorNormalize( studioAPI.lightVector[ i ] );
  }
}

STUDIOAPI_EXPORT void StudioDrawPoints( ) { }
STUDIOAPI_EXPORT void StudioDrawHulls( ) { }
STUDIOAPI_EXPORT void StudioDrawAbsBBox( ) { }
STUDIOAPI_EXPORT void StudioDrawBones( ) { }

STUDIOAPI_EXPORT void StudioSetupSkin( void *ptexturehdr, int index ) { }
STUDIOAPI_EXPORT void StudioSetRemapColors( int top, int bottom ) { }
STUDIOAPI_EXPORT void StudioClientEvents( ) { }
STUDIOAPI_EXPORT int StudioAPI_GetForceFaceFlags( ) { return studioAPI.forceFaceFlags; }
STUDIOAPI_EXPORT void StudioAPI_SetForceFaceFlags( int flags ) { studioAPI.forceFaceFlags = flags; }

STUDIOAPI_EXPORT void StudioAPI_SetupRenderer( int kRenderMode ) {
  studiohdr_t *modelHeader = studioAPI.modelHeader;

  kRenderMode = M_bound( 0, kRenderMode, kRenderTransAdd );

  studioAPI.blendMode = ToBlendMode( kRenderMode );

  if ( modelHeader && modelHeader->flags & STUDIO_HAS_BONEINFO ) {
    mstudioboneinfo_t *boneInfo = reinterpret_cast< mstudioboneinfo_t *>(
      modelHeader +
      modelHeader->boneindex + modelHeader->numbones * sizeof( mstudiobone_t ) );

    for ( int i = 0; i < modelHeader->numbones; i++ )
      Matrix3x4_ConcatTransforms( studioAPI.worldTransform[ i ], studioAPI.boneTransf[ i ].boneTransf, boneInfo[ i ].poseToBone );
  }
}

STUDIOAPI_EXPORT void StudioAPI_RestoreRenderer( ) {
  Backend_StudioAPI_RestoreRenderer( );
}

STUDIOAPI_EXPORT void StudioAPI_SetChromePosition( ) { VectorCopy( render3D.position, studioAPI.chromePosition ); }

STUDIOAPI_EXPORT void GL_StudioDrawShadow( ) { }

STUDIOAPI_EXPORT void StudioAPI_SetRenderMode( int mode ) {
  studioAPI.blendMode = ToBlendMode( mode );
}

STUDIOAPI_EXPORT void StudioSetRenderamt( int iRenderamt ) { }
STUDIOAPI_EXPORT void StudioSetCullState( int iCull ) { }
STUDIOAPI_EXPORT void StudioRenderShadow( int iSprite, float *p1, float *p2, float *p3, float *p4 ) { }

void StudioAPI_Init( ) {
  for ( int i = 0; i < MAXSTUDIOBONES; i++ ) {
    boneTransf[ i ] = reinterpret_cast<float ***>( studioAPI.boneTransf[ i ].boneTransf );
    lightTransf[ i ] = reinterpret_cast<float ***>( studioAPI.boneTransf[ i ].lightTransf );
  }
}

GAME_EXPORT qboolean StudioAPI_FillAPI_( engine_studio_api_s *outAPI, r_studio_interface_s *pDefaultDraw ) {
  outAPI->GetTimes = &StudioAPI_GetTimes;
  outAPI->GetViewInfo = &StudioAPI_GetViewpassInfo;
  outAPI->GetModelCounters = &StudioAPI_GetModelCounters;

  outAPI->PlayerInfo = &StudioAPI_GetPlayerInfo;
  outAPI->GetPlayerState = &StudioAPI_GetPlayerState;
  outAPI->SetupPlayerModel = &StudioAPI_SetupPlayerModel;

  outAPI->StudioSetupModel = &StudioAPI_SetModelBodyPart;
  outAPI->SetRenderModel = &StudioAPI_SetCurrentModel;

  outAPI->StudioGetBoneTransform = &StudioAPI_GetBoneTransform;
  outAPI->StudioDrawBones = StudioDrawBones;

  outAPI->StudioSetupLighting = &StudioAPI_SetupLighting;
  outAPI->StudioEntityLight = StudioEntityLight;
  outAPI->StudioDynamicLight = StudioDynamicLight;
  outAPI->StudioGetLightTransform = &StudioAPI_GetLightTransform;

  outAPI->GL_StudioDrawShadow = GL_StudioDrawShadow; // stub
  outAPI->StudioRenderShadow = StudioRenderShadow;

  outAPI->GetForceFaceFlags = &StudioAPI_GetForceFaceFlags;
  outAPI->SetForceFaceFlags = &StudioAPI_SetForceFaceFlags;

  outAPI->SetupRenderer = &StudioAPI_SetupRenderer;
  outAPI->RestoreRenderer = &StudioAPI_RestoreRenderer;
  outAPI->StudioSetHeader = &StudioAPI_SetHeader;

  outAPI->StudioGetRotationMatrix = &StudioAPI_GetRotationMatrix;

  outAPI->GetCurrentEntity = &StudioAPI_GetCurrentEntity;

  outAPI->StudioCheckBBox = StudioAPI_CheckBoundingBox; // stub
  outAPI->StudioDrawPoints = &StudioAPI_RenderModel;
  outAPI->StudioDrawHulls = StudioDrawHulls;     // stub
  outAPI->StudioDrawAbsBBox = StudioDrawAbsBBox; // stub

  outAPI->StudioSetupSkin = StudioSetupSkin;
  outAPI->StudioSetRemapColors = StudioSetRemapColors;
  outAPI->StudioClientEvents = StudioClientEvents;
  outAPI->SetChromeOrigin = &StudioAPI_SetChromePosition;
  outAPI->GL_SetRenderMode = &StudioAPI_SetRenderMode;
  outAPI->StudioSetRenderamt = StudioSetRenderamt;
  outAPI->StudioSetCullState = StudioSetCullState;

  pDefaultDraw->version = STUDIO_INTERFACE_VERSION;
  pDefaultDraw->StudioDrawModel = &StudioAPI_DrawModel;
  pDefaultDraw->StudioDrawPlayer = &StudioAPI_DrawPlayer;

  StudioAPI_Init( );

  return true;
}
