
#include <stdlib.h>
#include <float.h>
#include <math.h>

#include "ref_api.h"
#include "xash3d_mathlib.h"

#include "rx_backend.h"
#include "rx_base.h"
#include "rx_cvars.h"
#include "rx_3d.h"
#include "rx_3d_entity.h"
#include "rx_3d_surface.h"
#include "rx_video.h"
#include "rx_triapi.h"
#include "rx_macros.h"
#include "rx_memmgr.h"

#include "xyrherris-mathlib.h"

rx_render3dctx_t render3D;

GAME_EXPORT void Render3D_Render3DSCENE_( ) {
  if ( VIEWPASS_IS_NORMAL_PASS )
    render3D.frameTime = refClient->time - refClient->oldtime;
  else
    render3D.frameTime = 0.f;

  render3D.sceneFrameCount++;

  render3D.unloadMap = false;

  render3D.currentSurfaceKey = 0;

  render3D.worldModel = WORLD_MODEL;

  render3D.modelTransfType = TRANSF_IDENTITY;

  render3D.entityAlpha = 1.f;

  MAT3X4ROW_IDENTITY( render3D.modelTransform );
  VectorCopy( render3D.position, render3D.modelOrigin );

  Backend_Render3D_ArenasInit( );
  Backend_Render3D_SyncCvars( );

  Render3D_CvarSyncNearFarZ( );

  Render3D_RenderWorldModel( );
  Render3D_Entity_RenderEntities( );

  Backend_Render3D_RenderFlush( );

  // engineFuncs->R_DrawWorldHull( );
}

GAME_EXPORT void Render3D_ClearScene_( ) {
  Render3D_Entity_ClearCurrentDrawList( );

  if ( engineFuncs->drawFuncs->R_ClearScene )
    engineFuncs->drawFuncs->R_ClearScene( );
}

void Render3D_InitScene( ) {
  entityArena.drawListIdx = 0;
  entityArena.drawList = &entityArena.drawStack[ entityArena.drawListIdx ];
}

void Render3D_SyncViewpassParams( const ref_viewpass_s* viewpass ) {
  VectorCopy( viewpass->vieworigin, render3D.position );
  VectorCopy( viewpass->viewangles, render3D.viewAngles );
  Vector4Copy( viewpass->viewport, render3D.viewport );

  // when fov changes and resolution didnt change, save the bool before sync fov values
  bool fovChanged = ( render3D.fovX != viewpass->fov_x || render3D.fovY != viewpass->fov_y ) && !videoctx.viewChanged;

  // only call on view changed function after sync fov values, the flag is before the sync
  if ( fovChanged ) {
    render3D.fovX = viewpass->fov_x;
    render3D.fovY = viewpass->fov_y;

    render3D.cotFOVX = 1 / tanf( ( render3D.fovX * M_PI_F ) / 360.f );
    render3D.cotFOVY = 1 / tanf( ( render3D.fovY * M_PI_F ) / 360.f );

    Render3D_OnViewChanged( );
  }

  render3D.viewpassFlags = viewpass->flags;
  render3D.params = 0;

  render3D.projCenterX = videoctx.width / 2;
  render3D.projCenterY = videoctx.height / 2;

  // compute forward, right and up frustum directions from camera angles
  AngleVectors(
    render3D.viewAngles,
    &render3D.angleVector[ ANGLEVEC_FORWARD_IDX ],
    &render3D.angleVector[ ANGLEVEC_RIGHT_IDX ],
    &render3D.angleVector[ ANGLEVEC_UP_IDX ] );

  Render3D_BuildScreenEdges( );
  Render3D_BuildFrustumPlanes( );
  Render3D_BuildMVP( );
}

void Render3D_OnViewChanged( ) {
  // calculate projection center, we go with center of the screen
  render3D.projCenterX = static_cast<float>( videoctx.width ) / 2.f;
  render3D.projCenterY = static_cast<float>( videoctx.height ) / 2.f;

  // compute projection scale
  render3D.projScaleX = render3D.projCenterX / tan( DEG2RAD( render3D.fovX ) / 2.f );
  render3D.projScaleY = render3D.projCenterY / tan( DEG2RAD( render3D.fovY ) / 2.f );

  // cache
  render3D.invProjScaleX = 1.f / render3D.projScaleX;
  render3D.invProjScaleY = 1.f / render3D.projScaleY;
}

float Render3D_WorldToScreen( float point[ 3 ], float model[ 3 ], float* outX, float* outY ) {
  float relative[ 3 ];
  VectorSubtract( point, model, relative );

  float rotated[ 3 ];
  Render3D_RotatePointToCam( relative, rotated );

  float invz = 1.f / rotated[ 2 ];

  if ( outX ) *outX = render3D.projCenterX + render3D.projScaleX * ( rotated[ 0 ] * invz );
  if ( outY ) *outY = render3D.projCenterY - render3D.projScaleY * ( rotated[ 1 ] * invz );

  // returns 1/z
  return invz;
}

void Render3D_TransformPoint( float* point, float* out ) {
  if ( render3D.modelTransfType == TRANSF_IDENTITY )
    memcpy( out, point, 3 * SIZEOF_FLOAT );

  else if ( render3D.modelTransfType == TRANSF_TRANSLATION )
    M_VectorAddv(
      point, render3D.modelTransform[ 3 ], render3D.modelTransform[ 7 ], render3D.modelTransform[ 11 ], out );

  else if ( render3D.modelTransfType == TRANSF_AFFINE )
    M_Mat3x4Row_MutiplyVector( point, render3D.modelTransform, out );
}

void Render3D_TransformAABB( float mins[ 3 ], float maxs[ 3 ], float mat[ 12 ], float outMins[ 3 ], float outMaxs[ 3 ] ) {
  if ( render3D.modelTransfType == TRANSF_TRANSLATION ) {
    outMins[ 0 ] += mat[ 3 ];
    outMins[ 1 ] += mat[ 7 ];
    outMins[ 2 ] += mat[ 11 ];
  }
  else if ( render3D.modelTransfType == TRANSF_AFFINE ) {
    M_Mat3x4Row_TransformAABB( mins, maxs, mat, outMins, outMaxs );
  }
  else {
    VectorCopy( mins, outMins );
    VectorCopy( maxs, outMaxs );
  }
}

void Render3D_BuildMVP( ) {
  float* right = &render3D.angleVector[ ANGLEVEC_RIGHT_IDX ];
  float* up = &render3D.angleVector[ ANGLEVEC_UP_IDX ];
  float* forward = &render3D.angleVector[ ANGLEVEC_FORWARD_IDX ];

  float nearZ = cvarMirrors.rx_camnearz.value;
  float farZ = cvarMirrors.rx_camfarz.value;

  float* position = render3D.position;
  float* view = render3D.view;
  float* projection = render3D.projection;

  view[ 0 ] = right[ 0 ];
  view[ 1 ] = up[ 0 ];
  view[ 2 ] = -forward[ 0 ];
  view[ 3 ] = 0.f;

  view[ 4 ] = right[ 1 ];
  view[ 5 ] = up[ 1 ];
  view[ 6 ] = -forward[ 1 ];
  view[ 7 ] = 0.f;

  view[ 8 ] = right[ 2 ];
  view[ 9 ] = up[ 2 ];
  view[ 10 ] = -forward[ 2 ];
  view[ 11 ] = 0.f;

  view[ 12 ] = -DotProduct( position, right );
  view[ 13 ] = -DotProduct( position, up );
  view[ 14 ] = DotProduct( position, forward );
  view[ 15 ] = 1.f;

  float deltaDepth = farZ - nearZ;

  projection[ 0 ] = render3D.cotFOVX;
  projection[ 5 ] = render3D.cotFOVY;

  // since the camera looks down +z, we dont flip sign
  projection[ 10 ] = -( farZ + nearZ ) / deltaDepth;
  projection[ 11 ] = -1.f;
  projection[ 14 ] = -( 2.f * farZ * nearZ ) / deltaDepth;

  M_Mat4x4Col_Multiply( projection, view, render3D.mvp );
}

void Render3D_Shutdown( ) {
  Backend_Render3D_ArenasShutdown( );

  Render3D_SurfaceArena_Shutdown( );

  render3D.loadedGameplay = false;
}

void Render3D_OnMapLoad( ) { }
