
#include "rx_base.h"
#include "rx_video.h"
#include "rx_3d.h"
#include "rx_memmgr.h"
#include "rx_logsutils.h"

#include "rxsw_3d_edge.h"

#include "xyrherris-mathlib.h"

rxsw_edgearena_t edgeArena;

bool Soft_Render3D_Edge_ClipEdge(
  float point1[ 3 ], float point2[ 3 ],
  float outpoint1[ 3 ], float outpoint2[ 3 ],
  rx_plane_t planes[], int planeCount ) {

  VectorCopy( point1, outpoint1 );
  VectorCopy( point2, outpoint2 );

  for ( int i = 0; i < planeCount; i++ ) {
    rx_plane_t* currPlane = &planes[ i ];

    float dot1 = DotProduct( outpoint1, currPlane->normal ) - currPlane->dist,
          dot2 = DotProduct( outpoint2, currPlane->normal ) - currPlane->dist;

    // both points is outside, dont emit
    if ( dot1 < 0 && dot2 < 0 ) return false;
    // in front of the plane, skip this plane
    if ( dot1 >= 0 && dot2 >= 0 ) continue;

    float t = dot1 / ( dot1 - dot2 );

    float clipped[ 3 ];

    clipped[ 0 ] = outpoint1[ 0 ] + ( outpoint2[ 0 ] - outpoint1[ 0 ] ) * t;
    clipped[ 1 ] = outpoint1[ 1 ] + ( outpoint2[ 1 ] - outpoint1[ 1 ] ) * t;
    clipped[ 2 ] = outpoint1[ 2 ] + ( outpoint2[ 2 ] - outpoint1[ 2 ] ) * t;

    if ( dot1 < 0.f )
      VectorCopy( clipped, outpoint1 );
    else
      VectorCopy( clipped, outpoint2 );
  }

  return true;
}

bool Soft_Render3D_Edge_BuildEdge( float v0[ 3 ], float v1[ 3 ], rxsw_edge_t* outEdge ) {
  float startXf, startYf;
  float endXf, endYf;

  Render3D_WorldToScreen( v0, render3D.position, &startXf, &startYf );
  Render3D_WorldToScreen( v1, render3D.position, &endXf, &endYf );

  if ( startYf == endYf ) return false;

  if ( startYf > endYf ) {
    float t;
    M_swap( startYf, endYf, t );
    M_swap( startXf, endXf, t );
  }

  float deltaYf = endYf - startYf;
  if ( fabsf( deltaYf ) < 1e-6f ) return false;

  float stepXf = ( endXf - startXf ) / deltaYf;

  int startYInt = ( int )M_floor( startYf );

  float yPrestep = ( startYInt + 0.5f ) - startYf;
  float adjustedStartXf = startXf + yPrestep * stepXf;

  int endYInt = ( int )M_floor( endYf );
  if ( endYf == ( float )( endYInt ) ) endYInt--;

  outEdge->startY = startYInt;
  outEdge->endY = endYInt;

  outEdge->startXFP = ( int )( adjustedStartXf * Q16_1F );
  outEdge->stepXFP = ( int )( stepXf * Q16_1F );

  return true;
}

int Soft_Render3D_Edge_EmitEdges( float v0[ 3 ], float v1[ 3 ], float v2[ 3 ], rxsw_edge_t* outEdge0, rxsw_edge_t* outEdge1, rxsw_edge_t* outEdge2 ) {
  int count = 0;

  count += ( int )( Soft_Render3D_Edge_BuildEdge( v0, v1, outEdge0 ) );
  count += ( int )( Soft_Render3D_Edge_BuildEdge( v1, v2, outEdge1 ) );
  count += ( int )( Soft_Render3D_Edge_BuildEdge( v2, v0, outEdge2 ) );

  return count;
}

bool Soft_Render3D_Edge_BuildEdgeP(
  float projx0, float projy0,
  float projx1, float projy1,
  rxsw_edge_t* outEdge ) {

  float startXf = projx0, startYf = projy0;
  float endXf = projx1, endYf = projy1;

  if ( startYf == endYf ) return false;

  if ( startYf > endYf ) {
    float t;
    M_swap( startYf, endYf, t );
    M_swap( startXf, endXf, t );
  }

  float deltaYf = endYf - startYf;
  if ( fabsf( deltaYf ) < 1e-6f ) return false;

  float stepXf = ( endXf - startXf ) / ( endYf - startYf );

  outEdge->startY = M_floor( startYf );
  outEdge->endY = Q_min( endYf, videoctx.height - 1 );

  outEdge->startXFP = startXf * Q16_1F;
  outEdge->stepXFP = stepXf * Q16_1F;

  return true;
}

int Soft_Render3D_Edge_EmitEdgesP(
  float projx0, float projy0,
  float projx1, float projy1,
  float projx2, float projy2,
  rxsw_edge_t* outEdge0, rxsw_edge_t* outEdge1, rxsw_edge_t* outEdge2 ) {

  int count = 0;
  count += ( int )( Soft_Render3D_Edge_BuildEdgeP( projx0, projy0, projx1, projy1, outEdge0 ) );
  count += ( int )( Soft_Render3D_Edge_BuildEdgeP( projx1, projy1, projx2, projy2, outEdge1 ) );
  count += ( int )( Soft_Render3D_Edge_BuildEdgeP( projx2, projy2, projx0, projy0, outEdge2 ) );

  return count;
}

void drawedge( rxsw_edge_t* edge ) {
  int startY = edge->startY;
  int endY = edge->endY;

  if ( startY >= endY ) return;

  int xFP = edge->startXFP;
  int stepXFP = edge->stepXFP;

  if ( edge->startY < startY ) {
    int skippedY = startY - edge->startY;
    xFP += stepXFP * skippedY;
  }

  startY = Q_max( startY, 0 );
  endY = Q_min( endY, videoctx.height - 1 );

  for ( int y = startY; y <= endY; y++ ) {
    int x1 = xFP >> Q16_SHIFT;
    int x2 = ( xFP + stepXFP ) >> Q16_SHIFT;

    int minx = x1;
    int maxx = x2;

    if ( minx > maxx ) {
      minx = x2;
      maxx = x1;
    }

    minx = Q_max( minx, 0 );
    maxx = Q_min( maxx, videoctx.width - 1 );

    if ( minx > maxx ) continue;

    int row = y * videoctx.width;
    for ( int px = minx; px <= maxx; px++ ) {
      int idx = row + px;
      videoctx.frameBuffer[ idx ] = 0xFFFFFF;
    }

    xFP += stepXFP;
  }
}