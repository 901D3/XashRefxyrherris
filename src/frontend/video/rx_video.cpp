
#include <stdlib.h>
#include <stdint.h>

#include "engine_stuff.h"

#include "rx_base.h"

#include "rx_video.h"
#include "rx_cvars.h"
#include "rx_triapi.h"

#include "rx_logsutils.h"
#include "xyrherris-mathlib.h"

#include "rx_memmgr.h"

#include "rx_macros.h"

rx_videoctx_t videoctx;

GAME_EXPORT void Video_Set2DMode_( int enable ) {
  videoctx.is2D = enable != 0;

  TriAPI_Immediate_Color4ub_( 0xFF, 0xFF, 0xFF, 0xFF );
}

void Video_Shutdown( ) {
  if ( !videoctx.initialized )
    return;

  Backend_Video_Shutdown( );
}

bool Video_Init( ) {
  videoctx.initialized = false;

  videoctx.frameBufferPtr = nullptr;

  refGlobals->width = Q_max( refGlobals->width, 128 );
  refGlobals->height = Q_max( refGlobals->height, 128 );

  videoctx.width = refGlobals->width;
  videoctx.height = refGlobals->height;

  int width = videoctx.width;
  int height = videoctx.height;

  if ( videoctx.rotation ) {
    width = videoctx.height;
    height = videoctx.width;
  }

  if ( !Backend_Video_Init( ) )
    return false;

  videoctx.initialized = true;

  return true;
}
