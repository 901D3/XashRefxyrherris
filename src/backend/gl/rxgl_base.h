
#pragma once

#include <GL/glew.h>
#include "rxgl_stuff.h"

#include "rx_defs.h"

#define GL21_CFGNAME  "ref_xashrefxyr_gl21"
#define GLES2_CFGNAME "ref_xashrefxyr_gles2"

#define MAX_SAVED_GL_EXT_COUNT 256

typedef struct {
  GLboolean blend;
  GLboolean cullFace;
  GLboolean depthTest;
  GLboolean scissorTest;
  GLboolean stencilTest;

  GLuint buffer;
  GLuint elementArrayBuffer;

  GLuint program;
  GLenum texture;

} rxgl_states;

extern rxgl_states glState;

extern int GL_GetBackendEnumerator( );
extern const char* GL_GetRendererConfigName( );

extern void GL_ClearScreen( );

extern void GL_StartFrame( );
extern void GL_EndFrame( );

extern const char* GL_GetVersion( );
extern const char* GL_GetGLSLVersion( );
extern const char* GL_GetRenderer( );
extern const char* GL_GetVendor( );
extern void GL_PrintVersion( );
extern void GL_PrintGLSLVersion( );
extern void GL_PrintRenderer( );
extern void GL_PrintVendor( );

extern void GL_SetState( GLenum cap, GLboolean enable );
extern void GL_BindBuffer( GLenum target, GLuint buffer );
extern void GL_UseProgram( GLuint id );

extern void GL_Shutdown( );
extern bool GL_Init( );

void GL_BuildVertices( float v[ 6 * 2 ], float x0, float y0, float x1, float y1 );
void GL_BuildTextureVertices( float v[ 6 * 2 ], int imageWidth, int imageHeight, float u0, float v0, float u1, float v1 );
void GL_BuildDrawTextureVertices(
  float v[ 6 * 2 * 2 ],
  int imageWidth, int imageHeight,
  float scaledWidth, float scaledHeight,
  float x, float y,
  float imageCropLeft, float imageCropTop, float imageCropRight, float imageCropBottom );

extern void GL_Draw2D_ArenaInit( );
extern bool GL_Draw2D_FillRectProgramInit( );
extern bool GL_Draw2D_DrawImageProgramInit( );
extern bool GL_Draw2D_BaseInit( );
