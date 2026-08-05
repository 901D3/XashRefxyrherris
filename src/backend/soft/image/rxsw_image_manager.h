
#pragma once

#include "rx_image.h"

extern void Soft_ImageMGR_LoadImage(rx_image_t *image, int width, int height);
extern void Soft_ImageMGR_FreeImage(rx_image_t *image);
extern bool Soft_ImageMGR_Init();
