
#include "xash3d_mathlib.h"

#include "rx_base.h"
#include "rx_3d.h"

#include "rx_logsutils.h"

#include "rx_memmgr.h"

int Render3DUtils_SutherlandHodgman( float inVertices[], int vertexCount, float outVertices[], rx_plane_t* plane ) {
  int outVertexCount = 0;

  // save last point
  int previdx = ( vertexCount - 1 ) * 3;
  float prevdist = DotProduct( &inVertices[ previdx ], plane->normal ) - plane->dist;
  bool prevIsOut = prevdist < 0;

  for ( int i = 0; i < vertexCount; i++ ) {
    // get the current vertex and calculate dist and bool
    // we will later save those to previous vertex, dist and bool
    int curridx = i * 3;

    float currdist = DotProduct( &inVertices[ curridx ], plane->normal ) - plane->dist;
    bool currIsOut = currdist < 0;

    // if one of both points is outside of the plane, clip it so it touches the plane
    if ( !currIsOut ) {
      if ( prevIsOut ) {
        float t = prevdist / ( prevdist - currdist );

        // this is the new valid point that touches the plane, meaning the oob point ealier is clipped to this point
        VectorSet( &outVertices[ outVertexCount * 3 ],
          inVertices[ previdx ] + ( inVertices[ curridx ] - inVertices[ previdx ] ) * t,
          inVertices[ previdx + 1 ] + ( inVertices[ curridx + 1 ] - inVertices[ previdx + 1 ] ) * t,
          inVertices[ previdx + 2 ] + ( inVertices[ curridx + 2 ] - inVertices[ previdx + 2 ] ) * t );

        outVertexCount++;
      }

      // point is inside, append it to the output vertices array
      VectorCopy( &inVertices[ curridx ], &outVertices[ outVertexCount * 3 ] );
      outVertexCount++;
    }
    else if ( !prevIsOut ) {
      float t = prevdist / ( prevdist - currdist );

      // this is the new valid point that touches the plane, meaning the oob point ealier is clipped to this point
      VectorSet( &outVertices[ outVertexCount * 3 ],
        inVertices[ previdx ] + ( inVertices[ curridx ] - inVertices[ previdx ] ) * t,
        inVertices[ previdx + 1 ] + ( inVertices[ curridx + 1 ] - inVertices[ previdx + 1 ] ) * t,
        inVertices[ previdx + 2 ] + ( inVertices[ curridx + 2 ] - inVertices[ previdx + 2 ] ) * t );

      outVertexCount++;
    }

    // if both checks is false, we just skip the current edge and go the next edge

    // now save the current as previous
    previdx = curridx;
    prevdist = currdist;
    prevIsOut = currIsOut;
  }

  return outVertexCount;
}

int Render3DUtils_SutherlandHodgmanSVertices( rx_svertex_t inSVertices[], int svertexCount, rx_svertex_t outSVertices[], rx_plane_t* plane ) {
  int outVertexCount = 0;

  // save last point
  int previdx = svertexCount - 1;
  float prevdist = DotProduct( inSVertices[ previdx ].position, plane->normal ) - plane->dist;
  bool prevIsOut = prevdist < 0;

  for ( int i = 0; i < svertexCount; i++ ) {
    int curridx = i;

    rx_svertex_t* prevSVertex = &inSVertices[ previdx ];
    rx_svertex_t* currSVertex = &inSVertices[ curridx ];

    float currdist = DotProduct( currSVertex->position, plane->normal ) - plane->dist;
    bool currIsOut = currdist < 0;

    if ( !currIsOut ) {
      if ( prevIsOut ) {
        Render3DUtils_LerpSVertex(
          &outSVertices[ outVertexCount ], prevSVertex, currSVertex,
          prevdist / ( prevdist - currdist ) );

        outVertexCount++;
      }

      memcpy( &outSVertices[ outVertexCount ], currSVertex, sizeof( rx_svertex_t ) );
      outVertexCount++;
    }
    else if ( !prevIsOut ) {
      Render3DUtils_LerpSVertex(
        &outSVertices[ outVertexCount ], prevSVertex, currSVertex,
        prevdist / ( prevdist - currdist ) );

      outVertexCount++;
    }

    previdx = curridx;
    prevdist = currdist;
    prevIsOut = currIsOut;
  }

  return outVertexCount;
}

int Render3DUtils_ClipPolygon(
  float inVertices[], int vertexCount,
  float outVertices[], int outVertexCount,
  rx_plane_t planes[], int planeCount ) {

  if ( !inVertices || !outVertices ) return 0;
  if ( !planes || planeCount == 0 ) return vertexCount;

  float* tmp1 = ( float* )( ENGINE_ALLOC( refctx.memPool, outVertexCount * 3 * SIZEOF_FLOAT ) );
  float* tmp2 = ( float* )( ENGINE_ALLOC( refctx.memPool, outVertexCount * 3 * SIZEOF_FLOAT ) );

  memcpy( tmp1, inVertices, vertexCount * 3 * SIZEOF_FLOAT );
  int count = vertexCount;

  for ( int i = 0; i < planeCount; i++ ) {
    count = Render3DUtils_SutherlandHodgman( tmp1, count, tmp2, &planes[ i ] );

    if ( count == 0 ) {
      ENGINE_FREE( tmp1 );
      ENGINE_FREE( tmp2 );

      return 0;
    }

    if ( count > outVertexCount ) {
      warnlog( "clipped vertex count %u exceeds output vertex capacity %u", count, outVertexCount );

      ENGINE_FREE( tmp1 );
      ENGINE_FREE( tmp2 );

      return 0;
    }

    memcpy( tmp1, tmp2, count * 3 * SIZEOF_FLOAT );
  }

  memcpy( outVertices, tmp1, count * 3 * SIZEOF_FLOAT );

  ENGINE_FREE( tmp1 );
  ENGINE_FREE( tmp2 );

  return count;
}

int Render3DUtils_ClipSPolygon(
  rx_svertex_t inSVertices[], int vertexCount,
  rx_svertex_t outSVertices[], int outVertexCount,
  rx_plane_t planes[], int planeCount ) {

  if ( !inSVertices || !outSVertices ) return 0;
  if ( !planes || planeCount == 0 ) return vertexCount;

  rx_svertex_t* tmp1 = ( rx_svertex_t* )( ENGINE_ALLOC( refctx.memPool, outVertexCount * sizeof( rx_svertex_t ) ) );
  rx_svertex_t* tmp2 = ( rx_svertex_t* )( ENGINE_ALLOC( refctx.memPool, outVertexCount * sizeof( rx_svertex_t ) ) );

  memcpy( tmp1, inSVertices, vertexCount * sizeof( rx_svertex_t ) );
  int count = vertexCount;

  for ( int i = 0; i < planeCount; i++ ) {
    count = Render3DUtils_SutherlandHodgmanSVertices( tmp1, count, tmp2, &planes[ i ] );

    if ( count == 0 ) {
      ENGINE_FREE( tmp1 );
      ENGINE_FREE( tmp2 );

      return 0;
    }

    if ( count > outVertexCount ) {
      warnlog( "clipped vertex count %u exceeds output vertex capacity %u", count, outVertexCount );

      ENGINE_FREE( tmp1 );
      ENGINE_FREE( tmp2 );

      return 0;
    }

    memcpy( tmp1, tmp2, count * sizeof( rx_svertex_t ) );
  }

  memcpy( outSVertices, tmp1, count * sizeof( rx_svertex_t ) );

  ENGINE_FREE( tmp1 );
  ENGINE_FREE( tmp2 );

  return count;
}

bool Render3DUtils_LiangBarsky( float p, float q, float* t0, float* t1 ) {
  if ( p == 0.f ) {
    if ( q < 0.f ) return false;
  }
  else {
    float div = q / p;

    if ( p < 0.f ) {
      if ( div > *t1 ) return false;
      if ( div > *t0 ) *t0 = div;
    }
    else {
      if ( div < *t0 ) return false;
      if ( div < *t1 ) *t1 = div;
    }
  }

  return true;
}

bool Render3DUtils_LiangBarskyRect(
  float* x0, float* y0, float* x1, float* y1,
  float xMin, float yMin, float xMax, float yMax ) {

  float t0 = 0.f;
  float t1 = 1.f;

  float dx = *x1 - *x0;
  float dy = *y1 - *y0;

  if ( !Render3DUtils_LiangBarsky( -dx, *x0 - xMin, &t0, &t1 ) )
    return false;

  if ( !Render3DUtils_LiangBarsky( dx, xMax - *x0, &t0, &t1 ) )
    return false;

  if ( !Render3DUtils_LiangBarsky( -dy, *y0 - yMin, &t0, &t1 ) )
    return false;

  if ( !Render3DUtils_LiangBarsky( dy, yMax - *y0, &t0, &t1 ) )
    return false;

  if ( t1 < 1.f ) {
    *x1 = *x0 + t1 * dx;
    *y1 = *y0 + t1 * dy;
  }

  if ( t0 > 0.f ) {
    *x0 += t0 * dx;
    *y0 += t0 * dy;
  }

  return true;
}

bool Render3DUtils_CullAABB( float mins[ 3 ], float maxs[ 3 ], rx_plane_t* plane ) {
  float v[ 3 ];

  VectorSet( v,
    plane->normal[ 0 ] >= 0.f ? maxs[ 0 ] : mins[ 0 ],
    plane->normal[ 1 ] >= 0.f ? maxs[ 1 ] : mins[ 1 ],
    plane->normal[ 2 ] >= 0.f ? maxs[ 2 ] : mins[ 2 ] );

  return DotProduct( plane->normal, v ) < plane->dist;
}

bool Render3DUtils_CullAABBWrapper( float mins[ 3 ], float maxs[ 3 ], rx_plane_t planes[], int planeCount ) {
  for ( int i = 0; i < planeCount; i++ ) {
    if ( Render3DUtils_CullAABB( mins, maxs, &planes[ i ] ) )
      return true;
  }

  return false;
}

int Render3DUtils_IsFullyInside( float vertices[], int vertexCount, rx_plane_t planes[], int planeCount ) {
  for ( int i = 0; i < planeCount; i++ ) {
    rx_plane_t* plane = &planes[ i ];

    int insideCount = 0;

    for ( int j = 0; j < vertexCount; j++ ) {
      if ( ( DotProduct( &vertices[ j * 3 ], plane->normal ) - plane->dist ) >= 0.0f )
        insideCount++;
    }

    if ( insideCount == 0 )
      return FULLY_OUTSIDE;

    if ( insideCount != vertexCount )
      return PARTIALLY_OUTSIDE;
  }

  return FULLY_INSIDE;
}

bool Render3DUtils_FaceCull( bool negate, float* model, float* normal, float dist ) {
  float dot = DotProduct( model, normal ) - dist;
  return ( !negate && ( dot < -BACKFACE_EPSILON ) || negate && ( dot > BACKFACE_EPSILON ) );
}

void Render3DUtils_CatmullClark3D( float vertices[], int vertexCount, float outVertices[][ 12 ] ) {
  float centroid[ 3 ] = { 0.f, 0.f, 0.f };

  for ( int i = 0; i < vertexCount; i++ ) {
    int i3 = i * 3;

    centroid[ 0 ] += vertices[ i3 + 0 ];
    centroid[ 1 ] += vertices[ i3 + 1 ];
    centroid[ 2 ] += vertices[ i3 + 2 ];
  }

  centroid[ 0 ] /= vertexCount;
  centroid[ 1 ] /= vertexCount;
  centroid[ 2 ] /= vertexCount;

  float mCurr[ 3 ];
  float mPrev[ 3 ];

  for ( int i = 0; i < vertexCount; i++ ) {
    int currIdx = i * 3;

    int prevIdx = i - 1;
    if ( prevIdx == -1 ) prevIdx = vertexCount - 1;
    prevIdx *= 3;

    int nextIdx = i + 1;
    if ( nextIdx == vertexCount ) nextIdx = 0;
    nextIdx *= 3;

    float* currVertices = &vertices[ currIdx ];

    mCurr[ 0 ] = ( currVertices[ 0 ] + vertices[ nextIdx + 0 ] ) * 0.5f;
    mCurr[ 1 ] = ( currVertices[ 1 ] + vertices[ nextIdx + 1 ] ) * 0.5f;
    mCurr[ 2 ] = ( currVertices[ 2 ] + vertices[ nextIdx + 2 ] ) * 0.5f;

    mPrev[ 0 ] = ( vertices[ prevIdx ] + currVertices[ 0 ] ) * 0.5f;
    mPrev[ 1 ] = ( vertices[ prevIdx + 1 ] + currVertices[ 1 ] ) * 0.5f;
    mPrev[ 2 ] = ( vertices[ prevIdx + 2 ] + currVertices[ 2 ] ) * 0.5f;

    float* dst = outVertices[ i ];

    dst[ 0 ] = currVertices[ 0 ];
    dst[ 1 ] = currVertices[ 1 ];
    dst[ 2 ] = currVertices[ 2 ];

    dst[ 3 ] = mCurr[ 0 ];
    dst[ 4 ] = mCurr[ 1 ];
    dst[ 5 ] = mCurr[ 2 ];

    dst[ 6 ] = centroid[ 0 ];
    dst[ 7 ] = centroid[ 1 ];
    dst[ 8 ] = centroid[ 2 ];

    dst[ 9 ] = mPrev[ 0 ];
    dst[ 10 ] = mPrev[ 1 ];
    dst[ 11 ] = mPrev[ 2 ];
  }
}
