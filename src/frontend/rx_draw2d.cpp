
#include <stdint.h>

#include "rx_backend.h"
#include "rx_image_manager.h"
#include "rx_triapi.h"
#include "rx_draw2d.h"
#include "rx_macros.h"

GAME_EXPORT void Draw2D_FillRect_( int renderMode, float x, float y, float width, float height, uint8_t r, uint8_t g, uint8_t b, uint8_t a ) {
  TriAPI_Immediate_Color4ub_( r, g, b, a );
  TriAPI_SetRenderMode_( renderMode );

  Backend_Draw2D_FillRect( x, y, width, height );
}

GAME_EXPORT void Draw2D_DrawStretchImageWrapper_(
  float x, float y,
  float scaledWidth, float scaledHeight,
  float imageCropLeft, float imageCropTop,
  float imageCropRight, float imageCropBottom,
  int imageIdx ) {

  if ( imageCropRight > 1.f || imageCropBottom > 1.f || scaledWidth < 1.f || scaledHeight < 1.f )
    return;

  rx_image_t *image = &imageMGR.imageList[ imageIdx ];

  Backend_Draw2D_DrawStretchImage(
    x, y,
    scaledWidth, scaledHeight,
    imageCropLeft * image->width, imageCropTop * image->height,
    imageCropRight * image->width, imageCropBottom * image->height,
    image );
}
