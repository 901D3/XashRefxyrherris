
#include "ref_api.h"

#include "engine_stuff.h"

#include "rx_base.h"
#include "rx_video.h"
#include "rx_cvars.h"
#include "rx_stats.h"

#include "rxgl_base.h"
#include "rxgl_stuff.h"
#include "rxgl_3d_surface.h"

#include "rx_logsutils.h"

#include "rx_macros.h"

rxgl_states glState;

BACKEND_EXPORT int GL_GetBackendEnumerator( ) { return BACKEND_GL; }

BACKEND_EXPORT const char *GL_GetRendererConfigName( ) {
#ifdef GLES2
  return GLES2_CFGNAME;
#elif defined( GL21 )
  return GL21_CFGNAME;
#else
  return "ref_??";
#endif
}

BACKEND_EXPORT void GL_ClearScreen( ) {
  glClearColor( 0.9176f, 0.4392f, 0.0627f, 1.f );
  glClear( GL_COLOR_BUFFER_BIT );
}

BACKEND_EXPORT void GL_StartFrame( ) {
  GL_ClearScreen( );
  glClear( GL_DEPTH_BUFFER_BIT );

  GL_Draw2D_ArenaInit( );
}

BACKEND_EXPORT void GL_EndFrame( ) {
  if ( videoctx.viewChanged ) {
    GL_Video_OnViewChange( );
    return;
  }

  // draw the remaining draw cmds
  GL_Draw2D_DrawImageFlush( );

#ifdef R_SPEEDS
  if ( cvarMirrors.r_speeds )
    R_SPEEDS_LOG( "img: %i, flushes: %i", imgDrawCmdCount, imgDrawCmdFlushCount );

  imgDrawCmdCount = 0;
  imgDrawCmdFlushCount = 0;
#endif

  engineFuncs->GL_SwapBuffers( );
}

const char *GL_GetVersion( ) { return reinterpret_cast<const char *>( glGetString( GL_VERSION ) ); }
const char *GL_GetGLSLVersion( ) { return reinterpret_cast<const char *>( glGetString( GL_SHADING_LANGUAGE_VERSION ) ); }
const char *GL_GetRenderer( ) { return reinterpret_cast<const char *>( glGetString( GL_RENDERER ) ); }
const char *GL_GetVendor( ) { return reinterpret_cast<const char *>( glGetString( GL_VENDOR ) ); }
void GL_PrintVersion( ) { printf( "%s\n", GL_GetVersion( ) ); }
void GL_PrintGLSLVersion( ) { printf( "%s\n", GL_GetGLSLVersion( ) ); }
void GL_PrintRenderer( ) { printf( "%s\n", GL_GetRenderer( ) ); }
void GL_PrintVendor( ) { printf( "%s\n", GL_GetVendor( ) ); }

void GL_Shutdown( ) { }

void GL_InitGLStates( ) {
  // wait for gpu to process everything before continue
  glFinish( );

  glClearColor( 0.9176f, 0.4392f, 0.0627f, 1.f );

  GLboolean b;
  glGetBooleanv( GL_BLEND, &b );
  glState.blend = b;
  glGetBooleanv( GL_CULL_FACE, &b );
  glState.cullFace = b;
  glGetBooleanv( GL_DEPTH_TEST, &b );
  glState.depthTest = b;
  glGetBooleanv( GL_SCISSOR_TEST, &b );
  glState.scissorTest = b;
  glGetBooleanv( GL_STENCIL_TEST, &b );
  glState.stencilTest = b;

  GL_SetState( GL_BLEND, false );
  GL_SetState( GL_DITHER, false );
  GL_SetState( GL_CULL_FACE, false );
  GL_SetState( GL_DEPTH_TEST, false );
  GL_SetState( GL_SCISSOR_TEST, false );
  GL_SetState( GL_STENCIL_TEST, false );

  glState.program = -1;
  glState.texture = -1;
}

void GL_SetState( GLenum cap, GLboolean enable ) {
  GLboolean *cacheTarget = nullptr;

  if ( cap == GL_BLEND )
    cacheTarget = &glState.blend;

  else if ( cap == GL_CULL_FACE )
    cacheTarget = &glState.cullFace;

  else if ( cap == GL_DEPTH_TEST )
    cacheTarget = &glState.depthTest;

  else if ( cap == GL_SCISSOR_TEST )
    cacheTarget = &glState.scissorTest;

  else if ( cap == GL_STENCIL_TEST )
    cacheTarget = &glState.stencilTest;

  else {
    if ( enable )
      glEnable( cap );
    else
      glDisable( cap );

    return;
  }

  if ( *cacheTarget == enable ) return;

  *cacheTarget = enable;

  if ( enable )
    glEnable( cap );
  else
    glDisable( cap );
}

void GL_BindBuffer( GLenum target, GLuint buffer ) {
  GLuint *cacheTarget = nullptr;

  if ( target == GL_ARRAY_BUFFER )
    cacheTarget = &glState.buffer;

  else if ( target == GL_ELEMENT_ARRAY_BUFFER )
    cacheTarget = &glState.elementArrayBuffer;

  else {
    glBindBuffer( target, buffer );
    return;
  }

  if ( *cacheTarget == buffer ) return;

  *cacheTarget = buffer;

  glBindBuffer( target, buffer );
}

void GL_UseProgram( GLuint id ) {
  if ( id != glState.program ) {
    glState.program = id;
    glUseProgram( id );
  }
}

bool GL_Init( ) {
  // create context before init glew
  GL_Video_Init( );

  const float glVer = atof( GL_GetVersion( ) );
  if ( glVer < 2.1f ) {
    M_HostError( "OpenGL version %f is less than %f", glVer, 2.1f );

    engineFuncs->R_Free_Video( );
    return false;
  }

  glewExperimental = GL_TRUE;

  GLenum glewInitR = glewInit( );
  if ( glewInitR != GLEW_OK ) {
    M_HostError( "glewInit failed, error: %s", glewGetErrorString( glewInitR ) );
    return false;
  }

  if ( !GL_Draw2D_BaseInit( ) )
    return false;

  if ( !GL_Render3D_BaseInit( ) )
    return false;

  GL_InitGLStates( ); // Force the driver to run completely synchronously on the main thread.
  // This forces any driver-side crash to happen immediately during your OpenGL call,
  // keeping your functions in the call stack!
  glEnable( GL_DEBUG_OUTPUT );
  glEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS );

  return true;
}
