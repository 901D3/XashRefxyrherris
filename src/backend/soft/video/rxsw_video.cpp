
#include <stdlib.h>
#include <stdint.h>

#include "engine_stuff.h"

#include "rx_base.h"
#include "rx_video.h"
#include "rx_cvars.h"
#include "rx_3d.h"
#include "rx_triapi.h"
#include "rx_blend.h"


#include "rxsw_video.h"

#include "rx_logsutils.h"
#include "xyrherris-mathlib.h"

#include "rx_memmgr.h"

#include "rx_macros.h"

void Soft_Video_Set2DMode( int enable ) {
}

bool Soft_Video_EngineCreateBuffer( ) {
  bool createBufferReturnValue = engineFuncs->SW_CreateBuffer(
    videoctx.width, videoctx.height, &videoctx.swVideoCtx.stride, &videoctx.swVideoCtx.bpp,
    &videoctx.swVideoCtx.rMask,
    &videoctx.swVideoCtx.gMask,
    &videoctx.swVideoCtx.bMask );

  if ( !createBufferReturnValue ) {
    dlog( "SW_CreateBuffer returned %i", createBufferReturnValue );
    return false;
  }

  videoctx.swVideoCtx.rShift = M_firstbit( videoctx.swVideoCtx.rMask );
  videoctx.swVideoCtx.gShift = M_firstbit( videoctx.swVideoCtx.gMask );
  videoctx.swVideoCtx.bShift = M_firstbit( videoctx.swVideoCtx.bMask );

  videoctx.swVideoCtx.rBits = M_countbits( videoctx.swVideoCtx.rMask );
  videoctx.swVideoCtx.gBits = M_countbits( videoctx.swVideoCtx.gMask );
  videoctx.swVideoCtx.bBits = M_countbits( videoctx.swVideoCtx.bMask );

  return createBufferReturnValue;
}

void Soft_Video_AllocateScreenBuffer( ) {
  if ( videoctx.frameBufferPtr )
    ENGINE_FREE( videoctx.frameBufferPtr );

  if ( !Soft_Video_EngineCreateBuffer( ) )
    return;

  videoctx.frameBufferPtr = ENGINE_ALLOC( refctx.memPool, videoctx.width * videoctx.height * SIZEOF_INT );
}

void Soft_Video_OnViewChange( ) {
  Soft_Video_AllocateScreenBuffer( );
}

void Soft_Video_Shutdown( ) {
  if ( videoctx.frameBufferPtr )
    ENGINE_FREE( videoctx.frameBufferPtr );

  videoctx.frameBufferPtr = nullptr;

  memset( &videoctx, 0, sizeof( videoctx ) );
}

BACKEND_EXPORT bool Soft_Video_Init( ) {
  bool initVideo = engineFuncs->R_Init_Video( REF_SOFTWARE );
  if ( !initVideo ) {
    dlog( "R_Init_Video(REF_SOFTWARE) returned false\n" );

    engineFuncs->R_Free_Video( );
    return false;
  }

  Soft_Video_AllocateScreenBuffer( );

  dlog(
    "\n"
    "  stride: %i, bytes per pixel: %i\n"
    "  rMask: 0x%X, gMask: 0x%X, bMask: 0x%X\n"
    "  rShift: %i, gShift: %i, bShift: %i\n"
    "  rBits: %i, gBits: %i, bBits: %i\n",
    videoctx.swVideoCtx.stride, videoctx.swVideoCtx.bpp,
    videoctx.swVideoCtx.rMask,
    videoctx.swVideoCtx.gMask,
    videoctx.swVideoCtx.bMask,
    videoctx.swVideoCtx.rShift,
    videoctx.swVideoCtx.gShift,
    videoctx.swVideoCtx.bShift,
    videoctx.swVideoCtx.rBits,
    videoctx.swVideoCtx.gBits,
    videoctx.swVideoCtx.bBits );

  triapi.blendMode = BLEND_NORMAL;

  return true;
}
