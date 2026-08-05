/**
 * ImageManager.h | ImageManager.cpp - image manager definitions and its management functions ONLY
 *
 * NOTE: image manager does not handle drawing
 */

#pragma once

#include <stdint.h>
#include <string.h>

#include "xash3d_types.h"
#include "com_model.h"
#include "ref_api.h"

#include "rx_image.h"

#include "rx_logsutils.h"

#define MAX_IMAGE_COUNT 8192

#define IMGLOADFL_GENERATE_MIPS ( 1 << 0 ) // generate mips when loading an image

typedef struct {
  rx_image_t imageList[ MAX_IMAGE_COUNT ];
  int imageCount;

  // some kind of gl like texture drawing?

  // 32 bound images because gl have 32 texture unit macros
  int texUnits[ 32 ];
  int lastBoundImageIdx;

  // IMGLOADFL_*
  unsigned int imageLoadFlags;

  int imagePixFmt;

  bool initialized;

} rx_imagemgr_t;

extern rx_imagemgr_t imageMGR;

extern unsigned int imageManagerDataPool;

extern void ImageMGR_BindImage_( int tmu, unsigned int imageIdx );
extern int ImageMGR_UpdateImage_( const char *name, rgbdata_t *rgbaData, texFlags_t flags, qboolean update );
extern int ImageMGR_LoadImage_( const char *name, const uint8_t buffer[], size_t size, int flags );
extern void ImageMGR_UpdateTexture_( int imageIdx, int cols, int rows, int width, int height, const uint8_t *buffer, pixformat_t fmt );
extern const uint8_t *ImageMGR_GetBaseImageData_( unsigned int imageIdx );
extern const char *ImageMGR_GetImageName_( unsigned int imageIdx );
extern int ImageMGR_FindImage_( const char *name );
extern int ImageMGR_CreateImageWithBuffer_( const char *name, int width, int height, const void *buffer, texFlags_t flags );
extern void ImageMGR_FreeImageWrapper_( unsigned int imageIdx );

extern void ImageMGR_ImageList( );
extern void ImageMGR_ImageListMinimal( );

extern void ImageMGR_CreateInternalTextures( );

extern void ImageMGR_UnloadBrushImages( model_t *model );

extern rx_image_t *ImageMGR_AllocateImage( const char *name );
extern void ImageMGR_AllocateImageData( rx_image_t *image, int width, int height );

extern void ImageMGR_UpdateImageName( rx_image_t *image, const char *name );

extern void ImageMGR_ClearImage( rx_image_t *image );

extern int ImageMGR_GetImagebyName( const char *name );
extern int ImageMGR_GetFreeSlot( );

extern bool ImageMGR_FreeImage( int imageIdx );

inline void ImageManager_InitImageParams( int imageIdx );
inline void ImageMGR_InitAllImageParams( );

extern void ImageMGR_Shutdown( );

extern bool ImageMGR_Init( );

inline bool ImageManager_CheckImageName( const char *name ) {
  if ( !name )
    return false;

  int length = strlen( name );

  if ( length >= IMAGE_NAME_LENGTH ) {
    dlog( "image name is too long | %s (%i)", name, length );
    return false;
  }

  return true;
}

inline void ImageManager_InitImageParams( int imageIdx ) {
  rx_image_t *image = &imageMGR.imageList[ imageIdx ];

  if ( !image->dataRGBA8 ) return;

  ImageMGR_BindImage_( XASH_TEXTURE0, imageIdx );
}

inline void ImageMGR_InitAllImageParams( ) {
  for ( int i = 0; i < imageMGR.imageCount; i++ )
    ImageManager_InitImageParams( i );
}
