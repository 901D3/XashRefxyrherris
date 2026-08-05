
#pragma once

#include <GL/glew.h>

// wrap to GL21 for GLES2 functions
#ifdef GLES2
  #define glClearDepth glClearDepthf
  #define glDepthRange glDepthRangef
#endif

#define GL_ERROR_BUFFER_SIZE 1024

#define A_POSITION_VAR     position
#define A_TEXTURECOORD_VAR textureCoord
#define A_COLOR_VAR        color
#define A_WASHCOLOR_VAR    washColor

#define A_POSITION     "a_Position"
#define A_TEXTURECOORD "a_TextureCoord"
#define A_COLOR        "a_Color"
#define A_WASHCOLOR    "a_WashColor"

#define U_COLOR_VAR       color
#define U_WASHCOLOR_VAR   washColor
#define U_BLENDMODE_VAR   blendMode
#define U_TEXTUREUNIT_VAR textureUnit

#define U_COLOR       "u_Color"
#define U_WASHCOLOR   "u_WashColor"
#define U_BLENDMODE   "u_BlendMode"
#define U_TEXTUREUNIT "u_TextureUnit"

#define M_GLProgSetAttr( p, attrVar, name )                           \
  ( p )->attr.attrVar = glGetAttribLocation( ( p )->programId, name )

#define M_GLProgSetUnif( p, unifVar, name )                            \
  ( p )->unif.unifVar = glGetUniformLocation( ( p )->programId, name )

#define M_CreateShaderProg( name, vertSrc, fragSrc )                           \
  GL_CreateFullProgram(                                                        \
    name "Vert", name "Frag", name "Prog",                                     \
    vertSrc, fragSrc,                                                          \
    DELETE_FRAGMENT_SHADER | DELETE_VERTEX_SHADER | DELETE_PROGRAM_WHEN_FAIL )

#define M_GLAttachShader( programId, vertShaderId, fragShaderId ) \
  {                                                               \
    glAttachShader( programId, vertShaderId );                    \
    glAttachShader( programId, fragShaderId );                    \
  }

#define M_GLAssert( )                   \
  {                                     \
    GLenum err = glGetError( );         \
    if ( err ) glerrlog( "0x%X", err ); \
  }

enum {
  // delete when program linking success
  DELETE_VERTEX_SHADER = 0,
  DELETE_FRAGMENT_SHADER = 1U << 0,

  // vice versa
  DELETE_VERTEX_SHADER_WHEN_FAIL = 1U << 1,
  DELETE_FRAGMENT_SHADER_WHEN_FAIL = 1U << 2,
  DELETE_PROGRAM_WHEN_FAIL = 1U << 3,
};

extern GLuint GL_CompileShader( const char *shaderName, const char *shaderSrc, GLenum shaderType );
extern GLuint GL_CreateProgram( const char *programName );
extern void GL_LinkShadersToProgram( const char *programName, GLuint programId, GLuint vertexShaderId, GLuint fragmentShaderId, unsigned int flags );
extern GLuint GL_CreateFullProgram( const char *vertShaderName, const char *fragShaderName, const char *progShaderName, const char *vertShaderSrc, const char *fragShaderSrc, unsigned int flags );
extern char *GL_GetShaderError( GLuint id );
extern char *GL_GetProgramError( GLuint id );
