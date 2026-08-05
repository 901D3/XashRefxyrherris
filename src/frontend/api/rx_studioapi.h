
#pragma once

#include "xash3d_types.h"
#include "studio.h"
#include "r_studioint.h"
#include "com_model.h"

typedef struct {
  char name[ MAX_OSPATH ];
  char modelName[ MAX_OSPATH ];

  model_t *model;

} rx_playermodel_t;

typedef struct {
  // for indexing model vertices
  short vertexIdx;

  // whatever it is
  short normalIdx;

  // for gl, normalize it to 0 and 1
  short texel[ 2 ];
} rx_studiotricmd_t;

typedef struct {
  unsigned int flags;
  mstudiomesh_t *mesh;
} rx_sortedmesh_t;

typedef struct {
  matrix3x4 boneTransf;
  matrix3x4 lightTransf;

} rx_studiobonetransf_t;

typedef struct {
  matrix3x4 rotationMatrix;

} rx_studiomodeltransf_t;

typedef struct {
  double time;
  double frameTime;
  int frameCount;
  int blendMode;

  float chromePosition[ 3 ];

  // rotation matrix for current studio entity
  matrix3x4 rotationMatrix;

  // bones and light transforms for current studio entity
  rx_studiobonetransf_t boneTransf[ MAXSTUDIOBONES ];

  rx_studiomodeltransf_t modelTransf;

  matrix3x4 worldTransform[ MAXSTUDIOBONES ];

  // this array holds model for players(aka other players' model)
  rx_playermodel_t playerModel[ MAX_CLIENTS ];

  float modelVertices[ MAXSTUDIOVERTS ];

  float lightValues[ MAXSTUDIOVERTS ];

  int forceFaceFlags;

  studiohdr_t *modelHeader;
  player_info_s *currPlayerInfo;

  mstudiobodyparts_t *bodyPart;
  mstudiomodel_t *subModel;

  rx_sortedmesh_t *modelMeshes[ MAXSTUDIOMESHES ];

  alight_t light;
  vec3_t lightVector[ MAXSTUDIOBONES ];

} rx_studioctx_t;

/*
## StudioAPI.cpp
*/
extern r_studio_interface_t *engineStudioDraw;

extern rx_studioctx_t studioAPI;

extern void StudioAPI_LoadModelImages_( model_t *model, void *data );

extern void StudioAPI_SetupTimings( );
extern float StudioAPI_EstimateFrame( cl_entity_t *clEntity, mstudioseqdesc_t *sequenceDesc, double time );

extern player_info_s *StudioAPI_GetPlayerInfo( int index );
extern entity_state_s *StudioAPI_GetPlayerState( int index );
extern void StudioAPI_GetTimes( int *frameCount, double *currentTime, double *previousTime );
extern void StudioAPI_GetViewpassInfo( float origin[], float upView[], float rightView[], float forwardView[] );
extern void StudioAPI_GetModelCounters( int **s, int **a );
extern float ****StudioAPI_GetBoneTransform( );
extern float ****StudioAPI_GetLightTransform( );
extern float ***StudioAPI_GetRotationMatrix( );
extern void StudioSetupModel( int bodypart, void **ppbodypart, void **ppsubmodel );
extern int StudioAPI_CheckBoundingBox( );
extern void StudioDynamicLight( cl_entity_s *ent, alight_s *plight );
extern void StudioEntityLight( alight_s *plight );
extern void StudioSetupLighting( alight_s *plighting );
extern void StudioDrawPoints( );
extern void StudioDrawHulls( );
extern void StudioDrawAbsBBox( );
extern void StudioDrawBones( );
extern void StudioSetupSkin( void *ptexturehdr, int index );
extern void StudioSetRemapColors( int top, int bottom );
extern void StudioClientEvents( );
extern int StudioAPI_GetForceFaceFlags( );
extern void StudioAPI_SetForceFaceFlags( int flags );
extern void StudioAPI_SetHeader( void *modelHeader );
extern void StudioAPI_SetupRenderer( int rendermode );
extern void StudioAPI_RestoreRenderer( );
extern void StudioAPI_SetChromePosition( );
extern void GL_StudioDrawShadow( );
extern void StudioAPI_SetRenderMode( int mode );
extern void StudioSetRenderamt( int iRenderamt );
extern void StudioSetCullState( int iCull );
extern void StudioRenderShadow( int iSprite, float *p1, float *p2, float *p3, float *p4 );
extern int StudioAPI_DrawModel( int flags );
extern int StudioAPI_DrawPlayer( int flags, entity_state_s *pplayer );

extern qboolean StudioAPI_FillAPI_( engine_studio_api_s *outAPI, r_studio_interface_s *pDefaultDraw );
extern void StudioAPI_SetDrawInterface_( r_studio_interface_s *pDraw );

/*
## Model.cpp
*/
extern void StudioAPI_SetupModelTransform( float position[], float angle[], bool lerpMovement, bool isPlayer, bool flipViewModel );
extern void StudioAPI_SetCurrentModel( model_s *model );
extern void StudioAPI_LoadModelImage( model_t *model, studiohdr_t *modelMetadata, mstudiotexture_t *modelTexture );
extern model_s *StudioAPI_SetupPlayerModel( int index );
extern void StudioAPI_SetModelBodyPart( int bodyParti, void **bodyPart, void **subModel );

extern void StudioAPI_SetupEntityBonesTransforms( cl_entity_t *clEntity );
extern void StudioAPI_CalcRotations( cl_entity_t *clEntity, mstudioseqdesc_t *sequenceDesc, mstudioanim_t *animationList, float frame, vec3_t *outpos, vec4_t *outq );
extern void StudioAPI_CalcBoneAdjust( byte *controller1, float mouthOpenValue, float *out );
void StudioAPI_FxTransform( cl_entity_t *clEntity, matrix3x4 transform );
extern void StudioAPI_RenderModelWrapper( );
extern void StudioAPI_RenderModel( );

/*
## Entity.cpp
*/
extern cl_entity_s *StudioAPI_GetCurrentEntity( );

extern void StudioAPI_DrawStudioEntityModel( );

extern void StudioAPI_DrawModelInternal( bool isPlayer, int studioFlags );
