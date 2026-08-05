
#include <math.h>

#include "crclib.h"

#include "rx_backend.h"
#include "rx_video.h"
#include "rx_image.h"
#include "rx_image_manager.h"
#include "rx_blend.h"
#include "rx_triapi.h"

#include "rxsw_video.h"

#include "rx_logsutils.h"
#include "xyrherris-mathlib.h"

#include "rx_macros.h"

BACKEND_EXPORT void Soft_Draw2D_FillRect( float destXStart, float destYStart, float width, float height ) {
  int blendMode = triapi.blendMode;
  int washColor = triapi.washColor;

  int washColorA = M_geta( washColor );
  if ( washColorA == 0 ) return;

  unsigned int *frameBuffer = videoctx.frameBuffer;

  if ( destXStart + width > videoctx.width ) width = videoctx.width - destXStart;
  if ( width <= 0 ) return;

  if ( destYStart + height > videoctx.height ) height = videoctx.height - destYStart;
  if ( height <= 0 ) return;

  bool isAddWash = blendMode == BLEND_ADD_WASH;
  bool isNormal = blendMode == BLEND_NORMAL;
  //  bool isLerpSrcAlpha   = blendMode == BLEND_LERP;
  bool isLerpWashAlpha = blendMode == BLEND_LERP_WASH;

  unsigned int washColorR = washColor & 0xFF;
  unsigned int washColorG = washColor & 0xFF00;
  unsigned int washColorB = washColor & 0xFF0000;

  int minx = Q_max( destXStart, 0 );
  int miny = Q_max( destYStart, 0 );

  int maxx = Q_min( destXStart + width, videoctx.width );
  int maxy = Q_min( destYStart + height, videoctx.height );

  if ( washColorA == 255 && isNormal ) {
    for ( int y = miny; y < maxy; y++ ) {
      int row = y * videoctx.width;

      for ( int x = minx; x < maxx; x++ ) frameBuffer[ row + x ] = washColor;
    }
  }
  else { // do blending
    for ( int y = miny; y < maxy; y++ ) {
      int row = y * videoctx.width;

      for ( int x = minx; x < maxx; x++ ) {
        int destIdx = row + x;
        unsigned int destPixel = frameBuffer[ destIdx ];

        if ( isAddWash ) {
          unsigned int destPixelR = destPixel & 0xFF;
          unsigned int destPixelG = destPixel & 0xFF00;
          unsigned int destPixelB = destPixel & 0xFF0000;

          washColorR = ( washColorR * washColorA ) >> 8;
          washColorG = ( washColorG * washColorA ) >> 8;
          washColorB = ( washColorB * washColorA ) >> 8;

          frameBuffer[ destIdx ] = Q_min( destPixelR + washColorR, 0xFF ) | ( Q_min( destPixelG + washColorR, 0xFF00 ) & 0xFF00 ) | ( Q_min( destPixelB + washColorR, 0xFF0000 ) & 0xFF0000 );
        }
        else if ( isLerpWashAlpha ) {
          unsigned int washColorRB = washColorR | washColorB;

          unsigned int destPixelRB = destPixel & 0xFF00FF;
          unsigned int destPixelG = destPixel & 0xFF00;

          unsigned int blendedRB = destPixelRB + ( ( washColorA * ( washColorRB - destPixelRB ) ) >> 8 );
          unsigned int blendedG = destPixelG + ( ( washColorA * ( washColorG - destPixelG ) ) >> 8 );

          frameBuffer[ destIdx ] = ( blendedRB & 0xFF00FF ) | ( blendedG & 0xFF00 );
        }
      }
    }
  }
}

void Soft_Draw2D_DrawImage(
  unsigned int imageDataBuffer[], int imageWidth, int imageHeight,
  int destXStart, int destYStart,
  int scaledWidth, int scaledHeight,
  int imageCropLeft, int imageCropTop,
  int imageCropRight, int imageCropBottom ) {

  unsigned int blendMode = triapi.blendMode;
  unsigned int washColor = triapi.washColor;

  // hoist
  int videoWidth = videoctx.width;
  int videoHeight = videoctx.height;
  unsigned int *frameBuffer = videoctx.frameBuffer;

  unsigned int washColorR = M_getr( washColor );
  unsigned int washColorG = M_getg( washColor );
  unsigned int washColorB = M_getb( washColor );
  unsigned int washColorA = M_geta( washColor );

  bool isAddWash = blendMode == BLEND_ADD_WASH;
  bool isNormal = blendMode == BLEND_NORMAL;
  //  bool isLerpSrcAlpha   = blendMode == BLEND_LERP;
  bool isLerpWashAlpha = blendMode == BLEND_LERP_WASH;

  bool washColorOpaqueWhite = washColorA == 255 && washColorR == 255 && washColorG == 255 && washColorB == 255;

  // calculate min and max positions so we dont have to to oob checks inside loops
  int minx = Q_max( destXStart, 0 );
  int maxx = Q_min( destXStart + scaledWidth, videoWidth );

  int miny = Q_max( destYStart, 0 );
  int maxy = Q_min( destYStart + scaledHeight, videoctx.height );

  int srcAccumXStep = ( ( imageCropRight - imageCropLeft ) << Q16_SHIFT ) / scaledWidth;
  int srcAccumYStep = ( ( imageCropBottom - imageCropTop ) << Q16_SHIFT ) / scaledHeight;

  int srcAccumXStart = ( minx - destXStart ) * srcAccumXStep;

  // absolute position
  for ( int y = miny, srcAccumY = ( miny - destYStart ) * srcAccumYStep;
    y < maxy;
    y++, srcAccumY += srcAccumYStep ) {

    // we map dest position to src position to do nearest neighbor

    int srcRowWithBase = imageCropLeft + ( imageCropTop + ( srcAccumY >> Q16_SHIFT ) ) * imageWidth;
    int row = y * videoWidth;

    // absolute position
    for ( int x = minx, srcAccumX = srcAccumXStart;
      x < maxx;
      x++, srcAccumX += srcAccumXStep ) {
      int destIdx = row + x;

      // nearest neighbor fetch
      unsigned int srcPixel = imageDataBuffer[ srcRowWithBase + ( srcAccumX >> Q16_SHIFT ) ];

      unsigned int srcA = M_geta( srcPixel );
      if ( srcA == 0 ) continue;

      if ( isNormal ) {
        if ( srcA == 255 ) {
          frameBuffer[ destIdx ] = srcPixel;
          continue;
        }

        unsigned int destPixel = frameBuffer[ destIdx ];

        unsigned int destPixelRB = ( destPixel & 0xFF ) | ( destPixel & 0xFF0000 );
        unsigned int destPixelG = destPixel & 0xFF00;

        unsigned int srcRB = ( srcPixel & 0xFF ) | ( srcPixel & 0xFF0000 );

        unsigned int blendedRB = destPixelRB + ( ( srcA * ( srcRB - destPixelRB ) ) >> 8 );
        unsigned int blendedG = destPixelG + ( ( srcA * ( ( srcPixel & 0xFF00 ) - destPixelG ) ) >> 8 );

        frameBuffer[ destIdx ] = ( blendedRB & 0xFF00FF ) | ( blendedG & 0xFF00 );
      }
      else {
        // early continue
        unsigned int washedA = M_BlendMultiply( washColorA, srcA );
        if ( washedA == 0 ) continue;

        bool washAOpaque = washedA == 255;

        // color washing
        unsigned int washedR = M_BlendMultiply( washColorR, M_getr( srcPixel ) );
        unsigned int washedG = M_BlendMultiply( washColorG, M_getg( srcPixel ) );
        unsigned int washedB = M_BlendMultiply( washColorB, M_getb( srcPixel ) );

        if ( isAddWash ) {
          if ( washedR == 0 && washedG == 0 && washedB == 0 )
            continue;

          unsigned int destPixel = frameBuffer[ destIdx ];

          unsigned int destPixelR = destPixel & 0xFF;
          unsigned int destPixelG = destPixel & 0xFF00;
          unsigned int destPixelB = destPixel & 0xFF0000;

          washedG <<= 8;
          washedB <<= 16;

          washedR = ( washedR * washedA ) >> 8;
          washedG = ( washedG * washedA ) >> 8;
          washedB = ( washedB * washedA ) >> 8;

          frameBuffer[ destIdx ] =
            ( Q_min( destPixelB + washedB, 0xFF0000 ) & 0xFF0000 ) |
            ( Q_min( destPixelG + washedG, 0xFF00 ) & 0xFF00 ) |
            Q_min( destPixelR + washedR, 0xFF );
        }
        else if ( isLerpWashAlpha ) {
          if ( washAOpaque ) {
            // slap the pixel directly if opaque and wash color is full white
            if ( washColorOpaqueWhite ) {
              frameBuffer[ destIdx ] = srcPixel;
              continue;
            }

            frameBuffer[ destIdx ] = M_packr( washedR ) | M_packg( washedG ) | M_packb( washedB );
            continue;
          }

          unsigned int destPixel = frameBuffer[ destIdx ];

          unsigned int destPixelRB = destPixel & 0xFF00FF;
          unsigned int destPixelG = destPixel & 0xFF00;

          unsigned int washedRB = washedR | ( washedB << 16 );

          unsigned int blendedRB = destPixelRB + ( ( washedA * ( washedRB - destPixelRB ) ) >> 8 );
          unsigned int blendedG = destPixelG + ( ( washedA * ( ( washedG << 8 ) - destPixelG ) ) >> 8 );

          frameBuffer[ destIdx ] = ( blendedRB & 0xFF00FF ) | ( blendedG & 0xFF00 );
        }
      }
    }
  }
}

BACKEND_EXPORT void Soft_Draw2D_DrawStretchImage(
  int x, int y, int scaledWidth, int scaledHeight,
  int imageCropLeft, int imageCropTop,
  int imageCropRight, int imageCropBottom,
  rx_image_t *image ) {

  Soft_Draw2D_DrawImage(
    image->dataRGBA8, image->width, image->height,
    x, y,
    scaledWidth, scaledHeight,
    imageCropLeft, imageCropTop,
    imageCropRight, imageCropBottom );
}
