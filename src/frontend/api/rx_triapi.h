
#pragma once

#include "triangleapi.h"
#include "render_api.h"

enum {
  TRIVERT_VX = 0,
  TRIVERT_VY,
  TRIVERT_VZ,

  TRIVERT_U,
  TRIVERT_V,

  TRIVERT_R,
  TRIVERT_G,
  TRIVERT_B,
  TRIVERT_A,

  TRIVERT_ATTRCOUNT
};

typedef struct {
  int blendMode;
  float blendValue;

  int washColor;
  float washColorf[ 4 ];

  // float lightColorf[4];

  // bounded by GL_Bind
  int imageIdx;

  // fog stuff
  bool fogEnabled;

  float fogStart;
  float fogEnd;

  float fogColorf[ 4 ];
  float fogDensity;

  bool fogSkyboxImageIdx;

  // trictx arena / immediate mode

  bool immediateMode;

  // draw mode
  int drawMode;

  // rather than having vertices only pool, we also add u,v and r,g,b,a so it becomes
  // x,y,z, u,v, r,g,b,a, 9 components
  float *triVertexList;

  // we must multiply by 9 when using the count
  int triVertexCount;

  // blendMode in videoctx may change during adding vertices so we save the blend mode when triapi being
  // must not be confused with drawMode
  // blend mode is fixed when adding vertices by design, we dont save blend mode to TriAPIVertex
  int immediateBlendMode;

  bool initialized;

} rx_triapi_t;

extern rx_triapi_t triapi;

extern void TriAPI_SetRenderMode_( int blendMode );
extern void TriAPI_Immediate_Color4f_( float r, float g, float b, float a );
extern void TriAPI_Immediate_Color4ub_( unsigned char r, unsigned char g, unsigned char b, unsigned char a );

extern void TriAPI_Immediate_Begin_( int drawMode );
extern void TriAPI_Immediate_End_( );
extern void TriAPI_Immediate_AddVertex3f_( float x, float y, float z );
extern void TriAPI_Immediate_AddVertex3fv_( const float position[] );
extern void TriAPI_CullFace_( TRICULLSTYLE cullMode );

extern void TriAPI_Immediate_AddColor( );

extern void TriAPI_SetFog( float fogColorf[ 3 ], float startf, float endf, int bOn );
extern void TriAPI_3D_SetFogParms( float fogDensity, int fogSkyboxImageIdx );

extern void TriAPI_3D_ScreenToWorld( const vec3_t screen, vec3_t point );
extern void TriAPI_Immediate_AddTextureCoord2f( float u, float v );
extern void TriAPI_GetMatrix( const int pname, float *matrix );

extern void TriAPI_SetBlendMode( int blendMode );

extern void TriAPI_ArenaInit( );
extern void TriAPI_ArenaShutdown( );

extern bool TriAPI_Init( );

extern void TriAPI_FillAPI_( triangleapi_s *api );
