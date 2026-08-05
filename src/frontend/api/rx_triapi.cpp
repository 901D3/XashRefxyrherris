
#include <stdlib.h>
#include <stdint.h>

#include "xash3d_types.h"
#include "triangleapi.h"

extern "C" {
#include "xash3d_mathlib.h"
}

#include "rx_base.h"

#include "rx_backend.h"
#include "rx_3d.h"
#include "rx_cvars.h"
#include "rx_triapi.h"
#include "rx_video.h"
#include "rx_blend.h"
#include "rx_stats.h"

#include "rx_logsutils.h"
#include "xyrherris-mathlib.h"

#include "rx_macros.h"

#include "rx_memmgr.h"

#define MAX_TRIAPI_VERTEX_COUNT 256

rx_triapi_t triapi;

GAME_EXPORT void TriAPI_SetRenderMode_( int renderMode ) {
  triapi.blendMode = ToBlendMode( renderMode );
  Backend_TriAPI_SetBlendMode( triapi.blendMode );
}

GAME_EXPORT void TriAPI_Immediate_Begin_( int drawMode ) {
  if ( triapi.immediateMode )
    warnlog( "previous immediate instance is not ended, flushing the immediate instance" );

  TriAPI_ArenaInit( );
  triapi.drawMode = drawMode;

  Backend_TriAPI_Immediate_Begin( drawMode );
  triapi.immediateMode = false;

  // save blend mode when begin
  triapi.immediateBlendMode = triapi.blendMode;
}

GAME_EXPORT void TriAPI_Immediate_End_( ) {
  if ( !triapi.immediateMode ) return;

  Backend_TriAPI_Immediate_End( );

  triapi.immediateMode = false;
}

TRIAPI_EXPORT void TriAPI_Immediate_AddTextureCoord2f( float u, float v ) {
  if ( triapi.triVertexCount >= MAX_TRIAPI_VERTEX_COUNT )
    return;

  float *triVertex = &triapi.triVertexList[ triapi.triVertexCount * TRIVERT_ATTRCOUNT ];

  triVertex[ TRIVERT_U ] = u;
  triVertex[ TRIVERT_V ] = v;
}

GAME_EXPORT void TriAPI_Immediate_AddVertex3f_( float x, float y, float z ) {
  if ( triapi.triVertexCount >= MAX_TRIAPI_VERTEX_COUNT )
    return;

  float *triVertex = &triapi.triVertexList[ triapi.triVertexCount * TRIVERT_ATTRCOUNT ];

  triVertex[ TRIVERT_VX ] = x;
  triVertex[ TRIVERT_VY ] = y;
  triVertex[ TRIVERT_VZ ] = z;

  TriAPI_Immediate_AddColor( );

  // add vertex is called after add texture coord so we increment the vertexCount
  triapi.triVertexCount++;
}

GAME_EXPORT void TriAPI_Immediate_AddVertex3fv_( const float position[] ) { TriAPI_Immediate_AddVertex3f_( position[ 0 ], position[ 1 ], position[ 2 ] ); }

GAME_EXPORT void TriAPI_Immediate_Color4f_( float r, float g, float b, float a ) {
  int rInt = static_cast<int>( r * 255.f );
  int gInt = static_cast<int>( g * 255.f );
  int bInt = static_cast<int>( b * 255.f );
  int aInt = static_cast<int>( a * 255.f );

  triapi.washColor = M_packr( rInt ) | M_packg( gInt ) | M_packb( bInt ) | M_packa( aInt );

  Vector4Set( triapi.washColorf, r, g, b, a );

  if ( triapi.immediateMode ) TriAPI_Immediate_AddColor( );
}

GAME_EXPORT void TriAPI_Immediate_Color4ub_( unsigned char r, unsigned char g, unsigned char b, unsigned char a ) {
  triapi.washColor = M_packr( r ) | M_packg( g ) | M_packb( b ) | M_packa( a );

  float rf = r / 255.f;
  float gf = g / 255.f;
  float bf = b / 255.f;
  float af = a / 255.f;

  Vector4Set( triapi.washColorf, rf, gf, bf, af );

  if ( triapi.immediateMode ) TriAPI_Immediate_AddColor( );
}

GAME_EXPORT void TriAPI_CullFace_( TRICULLSTYLE cullMode ) { Backend_TriAPI_CullFace( cullMode ); }

void TriAPI_Immediate_AddColor( ) {
  if ( triapi.triVertexCount >= MAX_TRIAPI_VERTEX_COUNT ) return;

  float *triVertex = &triapi.triVertexList[ triapi.triVertexCount * TRIVERT_ATTRCOUNT ];

  triVertex[ TRIVERT_R ] = triapi.washColorf[ 0 ];
  triVertex[ TRIVERT_G ] = triapi.washColorf[ 1 ];
  triVertex[ TRIVERT_B ] = triapi.washColorf[ 2 ];
  triVertex[ TRIVERT_A ] = triapi.washColorf[ 3 ];
}

TRIAPI_EXPORT void TriAPI_SetFog( float fogColorf[ 3 ], float fogStart, float fogEnd, int enabled ) {
  triapi.fogEnabled = !!enabled;
  if ( !enabled ) return;

  // invalid params, set fogCustom to false
  if ( fogEnd <= fogStart ) return;

  // passes fog color have range of [0, 256) so we normalize it
  triapi.fogColorf[ 0 ] = fogColorf[ 0 ] / 255.f;
  triapi.fogColorf[ 1 ] = fogColorf[ 1 ] / 255.f;
  triapi.fogColorf[ 2 ] = fogColorf[ 2 ] / 255.f;
  triapi.fogColorf[ 3 ] = 1.f; // opaque fog

  triapi.fogStart = fogStart;
  triapi.fogEnd = fogEnd;
}

TRIAPI_EXPORT void TriAPI_3D_SetFogParms( float fogDensity, int fogSkyboxImageIdx ) {
  triapi.fogDensity = fogDensity;
  triapi.fogSkyboxImageIdx = fogSkyboxImageIdx;
}

TRIAPI_EXPORT void TriAPI_3D_ScreenToWorld( const vec3_t screen, vec3_t point ) {
  if ( !screen || !point )
    return;

  matrix4x4 toWorld;

  matrix4x4 newMVP = {
    {    render3D.mvp[ 0 ],         render3D.mvp[ 1 ],         render3D.mvp[ 2 ],         render3D.mvp[ 3 ]},
    {render3D.mvp[ 1 * 4 ], render3D.mvp[ 1 * 4 + 1 ], render3D.mvp[ 1 * 4 + 2 ], render3D.mvp[ 1 * 4 + 3 ]},
    {render3D.mvp[ 2 * 4 ], render3D.mvp[ 2 * 4 + 1 ], render3D.mvp[ 2 * 4 + 2 ], render3D.mvp[ 2 * 4 + 3 ]},
    {render3D.mvp[ 3 * 4 ], render3D.mvp[ 3 * 4 + 1 ], render3D.mvp[ 3 * 4 + 2 ], render3D.mvp[ 3 * 4 + 3 ]},
  };

  Matrix4x4_Invert_Full( toWorld, newMVP );

  point[ 0 ] = screen[ 0 ] * toWorld[ 0 ][ 0 ] + screen[ 1 ] * toWorld[ 0 ][ 1 ] + screen[ 2 ] * toWorld[ 0 ][ 2 ] + toWorld[ 0 ][ 3 ];
  point[ 1 ] = screen[ 0 ] * toWorld[ 1 ][ 0 ] + screen[ 1 ] * toWorld[ 1 ][ 1 ] + screen[ 2 ] * toWorld[ 1 ][ 2 ] + toWorld[ 1 ][ 3 ];
  point[ 2 ] = screen[ 0 ] * toWorld[ 2 ][ 0 ] + screen[ 1 ] * toWorld[ 2 ][ 1 ] + screen[ 2 ] * toWorld[ 2 ][ 2 ] + toWorld[ 2 ][ 3 ];
  float w = screen[ 0 ] * toWorld[ 3 ][ 0 ] + screen[ 1 ] * toWorld[ 3 ][ 1 ] + screen[ 2 ] * toWorld[ 3 ][ 2 ] + toWorld[ 3 ][ 3 ];

  if ( w != 0.f ) VectorScale( point, 1.f / w, point );
}

TRIAPI_EXPORT void TriAPI_GetMatrix( const int pname, float *matrix ) { }

void TriAPI_SetBlendMode( int blendMode ) {
  triapi.blendMode = blendMode;
  Backend_TriAPI_SetBlendMode( triapi.blendMode );
}

void TriAPI_ArenaInit( ) {
  if ( triapi.triVertexList ) ENGINE_FREE( triapi.triVertexList );

  triapi.triVertexList = reinterpret_cast<float *>( ENGINE_ALLOC( refctx.memPool, MAX_TRIAPI_VERTEX_COUNT * TRIVERT_ATTRCOUNT * SIZEOF_FLOAT ) );
  triapi.triVertexCount = 0;
}

void TriAPI_ArenaShutdown( ) {
  if ( !triapi.initialized ) return;

  if ( triapi.triVertexList ) ENGINE_FREE( triapi.triVertexList );
  triapi.triVertexList = nullptr;

  triapi.triVertexCount = 0;
}

bool TriAPI_Init( ) {
  triapi.initialized = false;

  triapi.triVertexList = nullptr;

  if ( !Backend_TriAPI_Init( ) ) return false;

  triapi.initialized = true;

  return true;
}

GAME_EXPORT void TriAPI_FillAPI_( triangleapi_s *api ) {
  api->TexCoord2f = TriAPI_Immediate_AddTextureCoord2f;
  api->Fog = TriAPI_SetFog;
  api->ScreenToWorld = TriAPI_3D_ScreenToWorld;
  api->GetMatrix = TriAPI_GetMatrix;
  api->FogParams = TriAPI_3D_SetFogParms;
}
