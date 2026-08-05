
#pragma once

enum {
  BLEND_NORMAL = 0, // dest = src

  BLEND_ADD,      // dest = min(srcRGB * srcA + dest, 255)
  BLEND_ADD_WASH, // dest = min(washrgb(srcRGB) * washa(srcA) + dest, 255)

  BLEND_MULTIPLY,      // dest *= srcRGB * srcA
  BLEND_MULTIPLY_WASH, // dest *= washrgb(srcRGB) * washa(srcA)

  BLEND_LERP,      // dest = srcRGB * srcA + dest * (1 - srcA)
  BLEND_LERP_WASH, // dest = washrgb(srcRGB) * washa(srcA) + dest * (1 - washA(srcA))

  BLEND_COUNT,

  BLENDFL_ZTEST = 1U << 16,
};

extern unsigned int ToBlendMode( int kRenderMode );
extern unsigned int ToBlendModeSurface( int kRenderMode );
