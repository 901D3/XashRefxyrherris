
#include "rx_base.h"

#include "rx_studioapi.h"
#include "rx_blend.h"

#include "rxgl_stuff.h"
#include "rxgl_studioapi.h"

void GL_StudioAPI_RestoreRenderer( ) {
  if ( studioAPI.blendMode == BLEND_NORMAL ) GL_SetState( GL_BLEND, false );
}
