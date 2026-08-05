
#pragma once

#include "rx_image.h"

#define SOFT_CFGNAME "ref_xashrefxyr_sw"

extern int Soft_GetBackendEnumerator( );
extern const char* Soft_GetRendererConfigName( );

extern void Soft_ClearScreen( );

extern void Soft_StartFrame( );
extern void Soft_EndFrame( );

extern void Soft_InitAllArenas( );

extern bool Soft_Init( );
extern void Soft_Shutdown( );

/*
## Draw2D.cpp
*/
extern void Soft_Draw2D_FillRect( float destXStart, float destYStart, float width, float height );

extern void Soft_Draw2D_DrawImage(
  unsigned int imageDataBuffer[], int imageWidth, int imageHeight,
  int destXStart, int destYStart,
  int scaledWidth, int scaledHeight,
  int imageCropLeft, int imageCropTop,
  int imageCropRight, int imageCropBottom );

extern void Soft_Draw2D_DrawStretchImage(
  int x, int y, int scaledWidth, int scaledHeight,
  int imageCropLeft, int imageCropTop,
  int imageCropRight, int imageCropBottom,
  rx_image_t* image );
