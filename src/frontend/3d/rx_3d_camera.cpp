
#include <math.h>

#include "xash3d_mathlib.h"
#include "engine_stuff.h"

#include "rx_defs.h"

#include "rx_base.h"

#include "rx_3d.h"

#include "rx_cvars.h"

#include "xyrherris-mathlib.h"

void Render3D_BuildScreenEdges( ) {
  float* screenEdges = render3D.screenEdges;

  screenEdges[ SCREEN_LEFT_IDX ] = render3D.cotFOVX;
  screenEdges[ SCREEN_LEFT_IDX + 1 ] = 0.f;
  screenEdges[ SCREEN_LEFT_IDX + 2 ] = 1.f;

  screenEdges[ SCREEN_RIGHT_IDX ] = -render3D.cotFOVX;
  screenEdges[ SCREEN_RIGHT_IDX + 1 ] = 0.f;
  screenEdges[ SCREEN_RIGHT_IDX + 2 ] = 1.f;

  screenEdges[ SCREEN_TOP_IDX ] = 0.f;
  screenEdges[ SCREEN_TOP_IDX + 1 ] = -render3D.cotFOVY;
  screenEdges[ SCREEN_TOP_IDX + 2 ] = 1.f;

  screenEdges[ SCREEN_BOTTOM_IDX ] = 0.f;
  screenEdges[ SCREEN_BOTTOM_IDX + 1 ] = render3D.cotFOVY;
  screenEdges[ SCREEN_BOTTOM_IDX + 2 ] = 1.f;

  for ( int i = 0; i < SCREEN_SIDE_COUNT; i++ ) VectorNormalize( &screenEdges[ i * 3 ] );
}

void Render3D_BuildFrustumPlanes( ) {
  float* screenEdges = render3D.screenEdges;
  rx_plane_t* frustumPlanes = render3D.frustumPlanes;

  float* position = render3D.position;
  float* angleVector = render3D.angleVector;

  for ( int i = 0; i < SCREEN_SIDE_COUNT; i++ ) {
    rx_plane_t* frustumPlane = &frustumPlanes[ i ];

    int screenEdgeIdx = i * 3;

    const float planeX = screenEdges[ screenEdgeIdx ];
    const float planeY = screenEdges[ screenEdgeIdx + 1 ];
    const float planeZ = screenEdges[ screenEdgeIdx + 2 ];

    frustumPlane->normal[ 0 ] = planeX * angleVector[ ANGLEVEC_RIGHT_IDX ] + planeY * angleVector[ ANGLEVEC_UP_IDX ] + planeZ * angleVector[ ANGLEVEC_FORWARD_IDX ];
    frustumPlane->normal[ 1 ] = planeX * angleVector[ ANGLEVEC_RIGHT_IDX + 1 ] + planeY * angleVector[ ANGLEVEC_UP_IDX + 1 ] + planeZ * angleVector[ ANGLEVEC_FORWARD_IDX + 1 ];
    frustumPlane->normal[ 2 ] = planeX * angleVector[ ANGLEVEC_RIGHT_IDX + 2 ] + planeY * angleVector[ ANGLEVEC_UP_IDX + 2 ] + planeZ * angleVector[ ANGLEVEC_FORWARD_IDX + 2 ];

    frustumPlane->dist = DotProduct( position, frustumPlane->normal );

    VectorNormalize( frustumPlane->normal );
  }
}

void Render3D_CvarSyncNearFarZ( ) {
  rx_plane_t* nearPlane = &render3D.frustumPlanes[ FRUSTUM_NEAR ];
  rx_plane_t* farPlane = &render3D.frustumPlanes[ FRUSTUM_FAR ];

  float* viewForward = &render3D.angleVector[ ANGLEVEC_FORWARD * ANGLEVEC_VECTOR_COUNT ];

  float nearZ = cvarMirrors.rx_camnearz.value;
  float farZ = cvarMirrors.rx_camfarz.value;

  float nearPoint[ 3 ];
  float farPoint[ 3 ];

  VectorMA( render3D.position, nearZ, viewForward, nearPoint );
  VectorMA( render3D.position, farZ, viewForward, farPoint );

  VectorCopy( viewForward, nearPlane->normal );
  VectorNegate( viewForward, farPlane->normal );

  VectorNormalize( nearPlane->normal );
  VectorNormalize( farPlane->normal );

  nearPlane->dist = DotProduct( nearPlane->normal, nearPoint );
  farPlane->dist = DotProduct( farPlane->normal, farPoint );
}
