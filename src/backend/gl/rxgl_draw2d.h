
#pragma once

#include "rxgl_stuff.h"

#ifdef R_SPEEDS
extern int imgDrawCmdCount;
extern int imgDrawCmdFlushCount;
#endif

extern void GL_Draw2D_FillRect( float x, float y, float width, float height );
extern void GL_Draw2D_DrawStretchImage(
  int x, int y, int scaledWidth, int scaledHeight,
  int imageCropLeft, int imageCropTop,
  int imageCropRight, int imageCropBottom,
  rx_image_t* image );

extern void GL_Draw2D_DrawImageFlush( );
