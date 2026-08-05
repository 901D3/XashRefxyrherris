
#include <stdlib.h>
#include <stdint.h>

#include "triangleapi.h"

#include "rx_cvars.h"
#include "rx_triapi.h"
#include "rx_image_manager.h"
#include "rx_blend.h"

#include "rxgl_base.h"
#include "rxgl_stuff.h"
#include "rxgl_triapi.h"

#include "rx_logsutils.h"

#include "rx_memmgr.h"

#include "rx_macros.h"

typedef struct {
  GLuint programId;

  struct {
    GLint position;
    GLint textureCoord;
    GLint washColor;
  } attr;

  struct {
    GLint textureUnit;
    GLint blendMode;
  } unif;

} rxgl_prog_tridraw;

static rxgl_prog_tridraw triDrawProgram;

static GLuint triAPIVBO;

static int glTriDrawMode;

BACKEND_EXPORT void GL_TriAPI_SetBlendMode( int blendMode ) {
  if ( blendMode == BLEND_NORMAL )
    GL_SetState( GL_BLEND, false );

  else {
    GL_SetState( GL_BLEND, true );
    glBlendEquation( GL_FUNC_ADD );

    if ( blendMode == BLEND_ADD || blendMode == BLEND_ADD_WASH )
      glBlendFunc( GL_SRC_ALPHA, GL_ONE );

    else if ( blendMode == BLEND_MULTIPLY || blendMode == BLEND_MULTIPLY_WASH )
      glBlendFunc( GL_ZERO, GL_SRC_COLOR );

    else if ( blendMode == BLEND_LERP || blendMode == BLEND_LERP_WASH )
      glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  }
}

BACKEND_EXPORT void GL_TriAPI_Immediate_Begin( int drawMode ) {
  if ( drawMode == TRI_TRIANGLES )
    glTriDrawMode = GL_TRIANGLES;

  else if ( drawMode == TRI_TRIANGLE_FAN || drawMode == TRI_QUADS || drawMode == TRI_POLYGON )
    glTriDrawMode = GL_TRIANGLE_FAN;

  else if ( drawMode == TRI_TRIANGLE_STRIP || drawMode == TRI_QUAD_STRIP )
    glTriDrawMode = GL_TRIANGLE_STRIP;

  else if ( drawMode == TRI_LINES )
    glTriDrawMode = GL_LINES;

  else if ( drawMode == TRI_POINTS )
    glTriDrawMode = GL_POINTS;
}

BACKEND_EXPORT void GL_TriAPI_Immediate_End( ) {
  GL_SetState( GL_DEPTH_TEST, true );
  glDepthFunc( GL_LESS );
  GL_TriAPI_SetBlendMode( triapi.immediateBlendMode );

  glUseProgram( triDrawProgram.programId );
  glBindBuffer( GL_ARRAY_BUFFER, triAPIVBO );
  glBufferData( GL_ARRAY_BUFFER, triapi.triVertexCount * 9 * SIZEOF_FLOAT, triapi.triVertexList, GL_STREAM_DRAW );

  rx_image_t *image = &imageMGR.imageList[ triapi.imageIdx ];
  if ( image->imageIdx == 0 ) return;

  glActiveTexture( GL_TEXTURE0 );
  glBindTexture( GL_TEXTURE_2D, image->glTexHandle );
  glUniform1i( triDrawProgram.unif.textureUnit, 0 );

  glEnableVertexAttribArray( triDrawProgram.attr.position );
  glVertexAttribPointer(
    triDrawProgram.attr.position,
    3,
    GL_FLOAT,
    GL_FALSE,
    9 * SIZEOF_FLOAT,
    reinterpret_cast<void *>( 0 ) );

  // u,v
  glEnableVertexAttribArray( triDrawProgram.attr.textureCoord );
  glVertexAttribPointer(
    triDrawProgram.attr.textureCoord,
    2,
    GL_FLOAT,
    GL_FALSE,
    9 * SIZEOF_FLOAT,
    reinterpret_cast<void *>( 3 * SIZEOF_FLOAT ) );

  // r,g,b,a
  glEnableVertexAttribArray( triDrawProgram.attr.washColor );
  glVertexAttribPointer(
    triDrawProgram.attr.washColor,
    4,
    GL_FLOAT,
    GL_FALSE,
    9 * SIZEOF_FLOAT,
    reinterpret_cast<void *>( 5 * SIZEOF_FLOAT ) );

  glUniform1i( triDrawProgram.unif.blendMode, triapi.immediateBlendMode );

  glDrawArrays( static_cast<GLenum>( glTriDrawMode ), 0, triapi.triVertexCount );
}

BACKEND_EXPORT void GL_TriAPI_CullFace( int cullMode ) { }

bool GL_TriAPI_ProgramInit( ) {
  const char *triDrawVert =
#include "rxgl_s_triapi_tridraw.vert.h"
    ;
  const char *triDrawFrag =
#include "rxgl_s_triapi_tridraw.frag.h"
    ;

  triDrawProgram.programId = M_CreateShaderProg( "TriDraw", triDrawVert, triDrawFrag );
  if ( !triDrawProgram.programId ) return false;

  M_GLProgSetAttr( &triDrawProgram, A_POSITION_VAR, A_POSITION );
  M_GLProgSetAttr( &triDrawProgram, A_TEXTURECOORD_VAR, A_TEXTURECOORD );
  M_GLProgSetAttr( &triDrawProgram, A_WASHCOLOR_VAR, A_WASHCOLOR );

  M_GLProgSetUnif( &triDrawProgram, U_TEXTUREUNIT_VAR, U_TEXTUREUNIT );
  M_GLProgSetUnif( &triDrawProgram, U_BLENDMODE_VAR, U_BLENDMODE );

  return true;
}

BACKEND_EXPORT bool GL_TriAPI_Init( ) {
  if ( !GL_TriAPI_ProgramInit( ) ) return false;

  glGenBuffers( 1, &triAPIVBO );

  return true;
}
