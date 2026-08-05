
#pragma once

#include <stdint.h>

#include "rx_macros.h"

typedef struct {
  union {
    unsigned int* scrBuffer;
  };

  unsigned int stride;
  unsigned int bpp; // BYTES per pixel

  unsigned int rMask, gMask, bMask;
  unsigned int rShift, gShift, bShift;
  unsigned int rBits, gBits, bBits;
} rxsw_videoctx_t;

typedef struct {
  // the buffer that we will draw and blit stuff to it, when end frame,
  // it will be copied to screen buffer to display on screen

  // hardcoded 4bpp (0xXXBBGGRR)
  union {
    void* frameBufferPtr;
    unsigned int* frameBuffer;
  };

  int width;
  int height;

  int rotation; // supports only value 0 and 1

  bool viewChanged;
  bool is2D;

  rxsw_videoctx_t swVideoCtx;

  bool initialized;

} rx_videoctx_t;

extern rx_videoctx_t videoctx;

extern void Video_Set2DMode_( int enable );

extern void Video_Shutdown( );
extern bool Video_Init( );
