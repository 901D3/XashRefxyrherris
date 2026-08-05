
#include "const.h"

#include "rx_blend.h"

unsigned int ToBlendMode( int kRenderMode ) {
  unsigned int blendMode;

  if ( kRenderMode == kRenderNormal )
    blendMode = BLEND_NORMAL;

  else if ( kRenderMode == kRenderTransColor || kRenderMode == kRenderTransAlpha )
    blendMode = BLEND_LERP;

  else if ( kRenderMode == kRenderTransTexture )
    blendMode = BLEND_LERP_WASH;

  else if ( kRenderMode == kRenderGlow )
    blendMode = BLEND_ADD | BLENDFL_ZTEST;

  else if ( kRenderMode == kRenderTransAdd )
    blendMode = BLEND_ADD_WASH;

  else
    blendMode = BLEND_NORMAL;

  return blendMode;
}

unsigned int ToBlendModeSurface( int kRenderMode ) {
  unsigned int blendMode;

  if ( kRenderMode == kRenderNormal )
    blendMode = BLEND_NORMAL;

  else if ( kRenderMode == kRenderTransColor || kRenderMode == kRenderTransTexture || kRenderMode == kRenderTransAlpha )
    blendMode = BLEND_LERP;

  else if ( kRenderMode == kRenderTransAdd )
    blendMode = BLEND_ADD;

  else if ( kRenderMode == kRenderGlow )
    blendMode = BLEND_ADD | BLENDFL_ZTEST;

  else
    blendMode = BLEND_NORMAL;

  return blendMode;
}
