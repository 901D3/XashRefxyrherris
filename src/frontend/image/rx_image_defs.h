
#pragma once

#include "xyrherris-types.h"

#define IMAGE_NAME_LENGTH 256

#define MAX_MIP_COUNT  4
#define MIP_ONLY_COUNT ( MAX_MIP_COUNT - 1 )

#define SKYBOX_SIDES 6

enum {
  PIXFMT_RGBA8 = 0, // 0xAABBGGRR
  PIXFMT_RGBA4,     // 0bRRRRGGGGBBBBAAAA
};

typedef struct rx_image_s rx_image_t;

struct rx_image_s {
  char name[ IMAGE_NAME_LENGTH ];
  int nameHash;
  int imageIdx; // if 0 then the image slot if unused or free to use

  // water textures
  unsigned int fogColorRGBA;

  unsigned int rgbDataFlags;
  unsigned int textureFlags;

  // PIXFMT_*
  int pixFmt;

  int width, height;

  // for gl, we will free the data because the data is uploaded to gpu
  // but we keep width and height
  unsigned int glTexHandle;

  // is power of 2 texture, true if width and height is a power of 2 number, purely optimization
  bool isPOT;

  union {
    void *dataPtr;
    pixbyte_t *dataByte;
    pixrgba8_t *dataRGBA8;
    pixrgba4_t *dataRGBA4;
  };

  // mip size always decrease the further the index is
  // when theres a duplication, its guaranteed that the further mips after the duplication is also a duplicate
  // so when we free, once we reach a duplication, we can break
  struct rx_image_s *mips[ MIP_ONLY_COUNT ];
};
