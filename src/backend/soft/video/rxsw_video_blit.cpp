
#include "cvardef.h"

#include "engine_stuff.h"

#include "rx_video.h"
#include "rx_cvars.h"

#include "rxsw_video.h"

#include "rx_logsutils.h"

#include "rx_macros.h"

#include "xyrherris-mathlib.h"

#ifdef R_SPEEDS
  #include <chrono>

  #include "rx_stats.h"
#endif

/*
## Soft_Video_BlitScreen
*/
void Soft_Video_BlitScreen( ) {
  videoctx.swVideoCtx.scrBuffer = ( pixrgba8_t* )( engineFuncs->SW_LockBuffer( ) );

  if ( !videoctx.swVideoCtx.scrBuffer ) {
    Soft_Video_AllocateScreenBuffer( );
    return;
  }

  // rotation = 0
  Soft_Video_BlitToScreenNoRotate( );

  engineFuncs->SW_UnlockBuffer( );
}

/*
## Soft_Video_BlitToScreenNoRotate
*/
void Soft_Video_BlitToScreenNoRotate( ) {
#ifdef R_SPEEDS
  auto t0 = std::chrono::steady_clock::now( );
#endif

  // hoist
  rxsw_videoctx_t* videoScreen = &videoctx.swVideoCtx;

  const int frameWidth = videoctx.width;
  const int frameHeight = videoctx.height;
  const int screenStride = videoScreen->stride;

  pixrgba8_t* frameBuffer = videoctx.frameBuffer;
  pixrgba8_t* scrBuffer = videoScreen->scrBuffer;

  unsigned int rShift = videoScreen->rShift;
  unsigned int gShift = videoScreen->gShift;
  unsigned int bShift = videoScreen->bShift;

  for ( int y = 0; y < frameHeight; y++ ) {
    int frameBufferRow = y * frameWidth;
    int screenBufferRow = y * screenStride;

    for ( int x = 0; x < frameWidth; x++ ) {
      unsigned int frameBufferPixel = frameBuffer[ frameBufferRow + x ];
      // frameBuffer[ frameBufferRow + x ] = 0;

      scrBuffer[ screenBufferRow + x ] = ( M_getr( frameBufferPixel ) << rShift ) | ( M_getg( frameBufferPixel ) << gShift ) | ( M_getb( frameBufferPixel ) << bShift );
    }
  }

#ifdef R_SPEEDS
  if ( cvarMirrors.r_speeds ) {
    float t1 = ( static_cast<std::chrono::duration<unsigned int, std::nano>>( std::chrono::steady_clock::now( ) - t0 ) ).count( ) / 1000000000.f;
    R_SPEEDS_LOG( "blit: %u pix/s, %.4f ms", static_cast<unsigned int>( videoctx.width * videoctx.height / t1 ), t1 * 1000.f );
  }
#endif
}
