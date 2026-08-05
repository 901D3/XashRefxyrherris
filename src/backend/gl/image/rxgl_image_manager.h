
#pragma once

#include "rx_image.h"

extern void GL_ImageMGR_LoadImage(rx_image_t *image, int width, int height);
extern void GL_ImageMGR_FreeImage(rx_image_t *image);

extern bool GL_ImageMGR_Init();
