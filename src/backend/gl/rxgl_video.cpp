
#include <stdlib.h>
#include <stdint.h>

#include "engine_stuff.h"

#include "rx_video.h"

#include "rxgl_base.h"
#include "rxgl_video.h"

#include "rx_logsutils.h"
#include "xyrherris-mathlib.h"

#include "rx_macros.h"

BACKEND_EXPORT void GL_Video_Set2DMode( int enable ) {
  if ( enable )
    GL_SetState( GL_DEPTH_TEST, false );
  else
    GL_SetState( GL_DEPTH_TEST, true );
}

BACKEND_EXPORT void GL_Video_OnViewChange( ) { glViewport( 0, 0, videoctx.width, videoctx.height ); }
BACKEND_EXPORT void GL_Video_Shutdown( ) { }

BACKEND_EXPORT bool GL_Video_Init( ) {
  bool initVideo = engineFuncs->R_Init_Video( REF_GL );
  if ( !initVideo ) {
    dlog( "R_Init_Video(REF_GL) returned false\n" );

    engineFuncs->R_Free_Video( );
    return false;
  }

  return true;
}
