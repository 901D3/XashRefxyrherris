
#pragma once

#include <stdint.h>

#include "com_image.h"

#include "rx_image_defs.h"

extern bool Image_RGBDataToImage( rx_image_t *destImage, rgbdata_t *rgbData );
extern void Image_GenerateMipsForImage( rx_image_t *image );
