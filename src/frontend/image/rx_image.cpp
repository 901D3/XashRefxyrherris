
#include <stdint.h>

#include "com_image.h"

#include "rx_backend.h"
#include "rx_base.h"
#include "rx_cvars.h"
#include "rx_image.h"
#include "rx_image_manager.h"
#include "rx_memmgr.h"

#include "xyrherris-mathlib.h"

#define IMAGE_MIPNAMEFORMAT "$MIP%i:%s"

float scaledMipScale[ MIP_ONLY_COUNT ];
static float baseMipScale[ MIP_ONLY_COUNT ] = { 1.f, 0.5f * 0.8f, 0.25f * 0.8f };

bool Image_RGBDataToImage( rx_image_t *outImage, rgbdata_t *rgbData ) {
  const int width = rgbData->width;
  const int height = rgbData->height;
  const int pixelCount = width * height;

  outImage->width = width;
  outImage->height = height;
  outImage->isPOT = M_IsPOTTexture( width, height );

  outImage->pixFmt = imageMGR.imagePixFmt;

  ImageMGR_AllocateImageData( outImage, width, height );

  const bool hasColor = rgbData->flags & IMAGE_HAS_COLOR;
  const bool hasAlpha = rgbData->flags & IMAGE_HAS_ALPHA;
  const bool isIndexed32 = rgbData->type == PF_INDEXED_32;
  const bool isIndexed24 = rgbData->type == PF_INDEXED_24;

  const bool isRGBA8 = outImage->pixFmt == PIXFMT_RGBA8;

  pixbyte_t *src = ( pixbyte_t * )( rgbData->buffer );
  pixrgba8_t *dstRGBA8 = outImage->dataRGBA8;
  pixrgba4_t *dstRGBA4 = outImage->dataRGBA4;

  // always expand to RGBA8888

  if ( rgbData->type == PF_RGBA_32 ) {
    for ( int i = 0; i < pixelCount; i++ ) {
      const int idx = i * 4;

      unsigned int r = src[ idx ],
                   g = src[ idx + 1 ],
                   b = src[ idx + 2 ],
                   a = hasAlpha ? src[ idx + 3 ] : 0xFF;

      if ( isRGBA8 )
        dstRGBA8[ i ] =
          r |
          ( g << 8 ) |
          ( b << 16 ) |
          ( a << 24 );

      else
        dstRGBA4[ i ] =
          ( ( r >> 4 ) << 12 ) |
          ( ( g >> 4 ) << 8 ) |
          ( ( b >> 4 ) << 4 ) |
          ( a >> 4 );
    }
  }
  else if ( isIndexed32 || isIndexed24 ) {
    const uint8_t *palette = rgbData->palette;

    for ( int i = 0; i < pixelCount; i++ ) {
      const int idx = i * 4;
      const int paletteIdx = src[ i ] * 4;

      unsigned int r = palette[ paletteIdx ],
                   g = palette[ paletteIdx + 1 ],
                   b = palette[ paletteIdx + 2 ],
                   a = ( hasAlpha && isIndexed32 ) ? palette[ paletteIdx + 3 ] : 0xFF;

      if ( isRGBA8 )
        dstRGBA8[ i ] =
          r |
          ( g << 8 ) |
          ( b << 16 ) |
          ( a << 24 );

      else
        dstRGBA4[ i ] =
          ( ( r >> 4 ) << 12 ) |
          ( ( g >> 4 ) << 8 ) |
          ( ( b >> 4 ) << 4 ) |
          ( a >> 4 );
    }
  }

  return true;
}

static inline void Image_GenerateMipRGBA8( pixrgba8_t *src, int width, int height, pixrgba8_t *dst ) {
  int dstWidth = Q_max( width / 2, 1 ),
      dstHeight = Q_max( height / 2, 1 );

  for ( int y = 0; y < dstHeight; y++ ) {
    for ( int x = 0; x < dstWidth; x++ ) {
      int x0 = x * 2,
          y0 = y * 2,
          x1 = Q_min( x0 + 1, width - 1 ),
          y1 = Q_min( y0 + 1, height - 1 );

      pixrgba8_t tl = src[ y0 * width + x0 ],
                 tr = src[ y0 * width + x1 ],
                 bl = src[ y1 * width + x0 ],
                 br = src[ y1 * width + x1 ];

      pixrgba8_t rb =
        ( ( tl & 0xFF00FF ) +
          ( tr & 0xFF00FF ) +
          ( bl & 0xFF00FF ) +
          ( br & 0xFF00FF ) ) >>
        2;

      pixrgba8_t ga =
        ( ( ( tl >> 8 ) & 0xFF00FF ) +
          ( ( tr >> 8 ) & 0xFF00FF ) +
          ( ( bl >> 8 ) & 0xFF00FF ) +
          ( ( br >> 8 ) & 0xFF00FF ) ) >>
        2;

      dst[ y * dstWidth + x ] = ( rb & 0xFF00FF ) | ( ( ga << 8 ) & 0xFF00FF00 );
    }
  }
}

static inline void Image_GenerateMipRGBA4( pixrgba4_t *src, int width, int height, pixrgba4_t *dst ) {
  int dstWidth = Q_max( width / 2, 1 ),
      dstHeight = Q_max( height / 2, 1 );

  for ( int y = 0; y < dstHeight; y++ ) {
    for ( int x = 0; x < dstWidth; x++ ) {
      int x0 = x * 2,
          y0 = y * 2,
          x1 = Q_min( x0 + 1, width - 1 ),
          y1 = Q_min( y0 + 1, height - 1 );

      pixrgba4_t tl = src[ y0 * width + x0 ],
                 tr = src[ y0 * width + x1 ],
                 bl = src[ y1 * width + x0 ],
                 br = src[ y1 * width + x1 ];

      pixrgba4_t rb =
        ( ( tl & 0b111100001111 ) +
          ( tr & 0b111100001111 ) +
          ( bl & 0b111100001111 ) +
          ( br & 0b111100001111 ) ) >>
        2;

      pixrgba4_t ga =
        ( ( ( tl >> 4 ) & 0b111100001111 ) +
          ( ( tr >> 4 ) & 0b111100001111 ) +
          ( ( bl >> 4 ) & 0b111100001111 ) +
          ( ( br >> 4 ) & 0b111100001111 ) ) >>
        2;

      dst[ y * dstWidth + x ] = ( rb & 0b111100001111 ) | ( ( ga << 4 ) & 0b1111000011110000 );
    }
  }
}

void Image_GenerateMipsForImage( rx_image_t *image ) {
  char mipName[ 256 ];

  rx_image_t *lastMip = image;
  int lastWidth = image->width,
      lastHeight = image->height;

  for ( int i = 0; i < MIP_ONLY_COUNT; i++ ) {
    int nextWidth = Q_max( lastWidth / 2, 1 ),
        nextHeight = Q_max( lastHeight / 2, 1 );

    if ( nextWidth == lastWidth && nextHeight == lastHeight ) {
      for ( ; i < MIP_ONLY_COUNT; i++ ) image->mips[ i ] = lastMip;
      break;
    }

    snprintf( mipName, sizeof( mipName ), IMAGE_MIPNAMEFORMAT, i + 1, image->name );

    rx_image_t *mip = ImageMGR_AllocateImage( mipName );
    mip->pixFmt = imageMGR.imagePixFmt;

    ImageMGR_AllocateImageData( mip, nextWidth, nextHeight );

    if ( mip->pixFmt == PIXFMT_RGBA8 )
      Image_GenerateMipRGBA8( lastMip->dataRGBA8, lastWidth, lastHeight, mip->dataRGBA8 );
    else
      Image_GenerateMipRGBA4( lastMip->dataRGBA4, lastWidth, lastHeight, mip->dataRGBA4 );

    Backend_ImageMGR_LoadImage( mip, nextWidth, nextHeight );

    image->mips[ i ] = mip;

    lastMip = mip;
    lastWidth = nextWidth;
    lastHeight = nextHeight;
  }
}
