
#include <stdint.h>

extern void Draw2D_FillRect_( int renderMode, float x, float y, float width, float height, uint8_t r, uint8_t g, uint8_t b, uint8_t a );
extern void Draw2D_DrawStretchImageWrapper_(
  float x, float y,
  float scaledWidth, float scaledHeight,
  float imageCropLeft, float imageCropTop,
  float imageCropRight, float imageCropBottom,
  int imageIdx );
