
#include <stdlib.h>
#include <string.h>

#include "xash3d_types.h"
#include "ref_api.h"
#include "render_api.h"
#include "com_image.h"
#include "crtlib.h"
extern "C" {
#include "crclib.h"
}
#include "engine_stuff.h"

#include "rx_backend.h"
#include "rx_base.h"
#include "rx_image.h"
#include "rx_image_manager.h"
#include "rx_video.h"
#include "rx_triapi.h"
#include "rx_logsutils.h"
#include "rx_memmgr.h"

#include "xyrherris-mathlib.h"

rx_imagemgr_t imageMGR;

GAME_EXPORT void ImageMGR_BindImage_( int texUnitNum, unsigned int imageIdx ) {
  if ( texUnitNum < 0 || texUnitNum >= 32 ) return;

  rx_image_t* image = &imageMGR.imageList[ imageIdx ];

  imageMGR.texUnits[ texUnitNum ] = imageIdx;

  imageMGR.lastBoundImageIdx = imageIdx;

  Backend_TriAPI_SetBlendMode( triapi.blendMode );
}

GAME_EXPORT int ImageMGR_LoadImage_( const char* name, const uint8_t buffer[], size_t size, int flags ) {
  if ( !ImageManager_CheckImageName( name ) )
    return 0;

  // getImageByName only return the image index but we want to check if the image really exist
  rx_image_t* image = &imageMGR.imageList[ ImageMGR_GetImagebyName( name ) ];
  if ( image->imageIdx )
    return image->imageIdx;

  unsigned int imageFlags = 0;

  // the flags passed to this function is for enabling some more params
  // example if flags has TF_NEAREST, then we will use nearest(in gl backend)

  if ( flags & TF_NOFLIP_TGA )
    imageFlags |= IL_DONTFLIP_TGA;

  if ( ( flags & TF_KEEP_SOURCE ) && !( flags & TF_EXPAND_SOURCE ) )
    imageFlags |= IL_KEEP_8BIT;

  engineFuncs->Image_SetForceFlags( imageFlags );

  rgbdata_t* rgbData = engineFuncs->FS_LoadImage( name, buffer, size );
  if ( !rgbData )
    return 0;

  image = ImageMGR_AllocateImage( name );
  if ( !image ) return 0;

  if ( !Image_RGBDataToImage( image, rgbData ) ) {
    ImageMGR_ClearImage( image );
    engineFuncs->FS_FreeImage( rgbData );
    return 0;
  }

  image->rgbDataFlags = rgbData->flags;
  image->textureFlags = flags;

  engineFuncs->FS_FreeImage( rgbData );

  // process some more data
  // for gl, we will upload image to gpu once and save the handle to gl side
  // since the image is on the gpu, we also free it
  Backend_ImageMGR_LoadImage( image, image->width, image->height );

  if ( imageMGR.imageLoadFlags & IMGLOADFL_GENERATE_MIPS )
    Image_GenerateMipsForImage( image );

  return image->imageIdx; // MUST return the index of the new added image
}

GAME_EXPORT int ImageMGR_UpdateImage_( const char* name, rgbdata_t* rgbData, texFlags_t flags, qboolean update ) {
  if ( !ImageManager_CheckImageName( name ) )
    return 0;

  if ( !rgbData ) {
    dlog( "%s couldn't load image data", name );
    return 0;
  }

  // Look up the image slot
  rx_image_t* image = &imageMGR.imageList[ ImageMGR_GetImagebyName( name ) ];

  if ( image->imageIdx != 0 && !update )
    return image->imageIdx;

  if ( image->imageIdx == 0 ) {
    if ( update ) {
      // trying to update a texture that doesn't exist
      M_HostError( "couldn't find texture \"%s\" for update", name );
      return 0;
    }

    image = ImageMGR_AllocateImage( name );
  }

  unsigned int imageFlags = 0;

  if ( rgbData->flags & IMAGE_PREMULTIPLIED ) imageFlags |= TF_PREMULTIPLIED;
  if ( rgbData->flags & IMAGE_HAS_LUMA ) imageFlags |= TF_HAS_LUMA;
  if ( rgbData->flags & IMAGE_QUAKEPAL ) imageFlags |= TF_QUAKEPAL;
  if ( flags & TF_MAKELUMA ) imageFlags |= IMAGE_MAKE_LUMA;

  image->rgbDataFlags |= imageFlags;

  Image_RGBDataToImage( image, rgbData );

  Backend_ImageMGR_LoadImage( image, image->width, image->height );

  // we dont free the passes rgbData here because renderer side dont own it

  return image->imageIdx;
}

GAME_EXPORT void ImageMGR_UpdateTexture_( int imageIdx, int cols, int rows, int width, int height, const uint8_t* buffer, pixformat_t fmt ) {
}

GAME_EXPORT const uint8_t* ImageMGR_GetBaseImageData_( unsigned int imageIdx ) {
  rx_image_t* image = &imageMGR.imageList[ imageIdx ];

  if ( !image || !image->dataRGBA8 )
    return nullptr;

  return ( uint8_t* )( image->dataRGBA8 );
}

GAME_EXPORT const char* ImageMGR_GetImageName_( unsigned int imageIdx ) {
  return imageMGR.imageList[ imageIdx ].name;
}

GAME_EXPORT int ImageMGR_FindImage_( const char* name ) {
  if ( !name ) return 0;

  const int nameHash = COM_HashKey( name, MAX_IMAGE_COUNT );

  for ( int i = 0; i < imageMGR.imageCount; i++ ) {
    if ( nameHash == imageMGR.imageList[ i ].nameHash )
      return imageMGR.imageList[ i ].imageIdx;
  }

  return 0;
}

GAME_EXPORT RENDERAPI_EXPORT int ImageMGR_CreateImageWithBuffer_( const char* name, int width, int height, const void* buffer, texFlags_t flags ) {
  if ( flags & TF_CUBEMAP ) {
    dlog( "image name %s with flag %i(TF_CUBEMAP) will not be created", name, TF_CUBEMAP );
    return 0;
  }

  int dataSize = 1;
  rgbdata_t rgbData;

  if ( flags & TF_ARB_16BIT )
    dataSize = 2;
  else if ( flags & TF_ARB_FLOAT )
    dataSize = 4;

  rgbData.width = width;
  rgbData.height = height;
  rgbData.size = width * height * dataSize * 4;
  rgbData.buffer = ( byte* )( ( void* )( buffer ) );

  flags = ( texFlags_t )( flags & ~( ( texFlags_t )( TF_TEXTURE_3D ) ) );

  return ImageMGR_UpdateImage_( name, &rgbData, flags, false );
}

GAME_EXPORT void ImageMGR_FreeImageWrapper_( unsigned int imageIdx ) {
  if ( imageIdx <= 0 ) return;

  ImageMGR_FreeImage( imageIdx );
}

GAME_EXPORT void ImageMGR_GetDetailScaleForImage_( int imageIdx, float* xScale, float* yScale ) {
  if ( xScale ) *xScale = 0.f;
  if ( yScale ) *yScale = 0.f;
}

GAME_EXPORT void ImageMGR_ProcessImage_( int imageIdx, float gamma, int topColor, int bottomColor ) {
  if ( imageIdx < 0 || imageIdx >= MAX_IMAGE_COUNT )
    return;

  rx_image_t* image = &imageMGR.imageList[ imageIdx ];
  unsigned int flags = 0;

  if ( gamma != 1.f )
    flags = IMAGE_LIGHTGAMMA;

  else if ( topColor != -1 && bottomColor != -1 )
    flags = IMAGE_REMAP;

  else {
    dlog( "bad operation for %s", image->name );
    return;
  }

  flags |= IMAGE_FORCE_RGBA;

  if ( !image->dataRGBA8 ) {
    dlog( "base image data is not allocated for \"%s\"", image->name );
    return;
  }

  rgbdata_t* rgbData = engineFuncs->FS_LoadImage( image->name, nullptr, 0 );
  engineFuncs->Image_Process( &rgbData, topColor, bottomColor, flags, 0.f );
  Image_RGBDataToImage( image, rgbData );
  image->rgbDataFlags = rgbData->flags;
  engineFuncs->FS_FreeImage( rgbData );
}

void ImageMGR_ImageList( ) {
  int totalSize = 0;

  dlog( "" );
  engineFuncs->Con_Printf( "<idx>           <dimension>                   <size> <fmt>        <type>       <name>\n" );

  rx_image_t* image = nullptr;
  for ( int imageIdx = 0; imageIdx < imageMGR.imageCount; imageIdx++ ) {
    image = &imageMGR.imageList[ imageIdx ];

    if ( image->name[ 0 ] == '\0' )
      continue;

    char imageFlagsType[ 32 ];

    if ( image->rgbDataFlags & TF_CLAMP )
      strncpy( imageFlagsType, "clamp", sizeof( imageFlagsType ) );

    else
      strncpy( imageFlagsType, "repeat", sizeof( imageFlagsType ) );

    int currSize = image->width * image->height * ( ( image->pixFmt == PIXFMT_RGBA8 ) ? 4 : 2 );

    totalSize += currSize;

    engineFuncs->Con_Printf(
      "%5i %15ix%-15i %14s %-12s %-12s %s\n",
      imageIdx,
      image->width, image->height,
      Q_memprint( currSize ),
      ( image->rgbDataFlags & TF_NORMALMAP ) ? "normal" : "diffuse",
      imageFlagsType,
      image->name );
  }

  engineFuncs->Con_Printf( "total size (image data only): %s", Q_memprint( totalSize ) );
}

void ImageMGR_ImageListMinimal( ) {
  int totalSize = 0;

  dlog( "" );
  engineFuncs->Con_Printf( "idx|dimension|size|fmt|type|name\n" );

  rx_image_t* image = nullptr;
  for ( int imageIdx = 0; imageIdx < imageMGR.imageCount; imageIdx++ ) {
    image = &imageMGR.imageList[ imageIdx ];
    if ( !image )
      continue;

    char imageFlagsType[ 32 ];

    if ( image->rgbDataFlags & TF_CLAMP )
      strncpy( imageFlagsType, "clamp", sizeof( imageFlagsType ) );

    else
      strncpy( imageFlagsType, "repeat", sizeof( imageFlagsType ) );

    int currSize = image->width * image->height * ( ( image->pixFmt == PIXFMT_RGBA8 ) ? 4 : 2 );

    totalSize += currSize;

    engineFuncs->Con_Printf(
      "%i|%ix%i|%s|%s|%s|%s\n",
      imageIdx,
      image->width, image->height,
      Q_memprint( currSize ),
      ( image->rgbDataFlags & TF_NORMALMAP ) ? "normal" : "diffuse",
      imageFlagsType,
      image->name );
  }

  engineFuncs->Con_Printf( "total size (image data only): %s", Q_memprint( totalSize ) );
}

void ImageMGR_CreateInternalTextures( ) {
  rx_image_t* image = ImageMGR_AllocateImage( "*default" );
  ImageMGR_AllocateImageData( image, 2, 2 );
  refctx.image.defaultImageIdx = image->imageIdx;

  for ( int y = 0; y < 2; y++ ) {
    for ( int x = 0; x < 2; x++ ) {
      if ( ( y < 2 ) ^ ( x < 2 ) )
        image->dataRGBA8[ y * 2 + x ] = 0xFFFF00FF;
      else
        image->dataRGBA8[ y * 2 + x ] = 0xFF000000;
    }
  }

  Backend_ImageMGR_LoadImage( image, image->width, image->height );

  image = ImageMGR_AllocateImage( "*particle" );
  ImageMGR_AllocateImageData( image, 16, 16 );
  refctx.image.particleImageIdx = image->imageIdx;

  for ( int y = 0; y < 16; y++ ) {
    int dy2 = y - 8;
    dy2 *= dy2;

    for ( int x = 0; x < 16; x++ ) {
      int dx = x - 8;

      image->dataRGBA8[ y * 16 + x ] = bound( 0, 255 - 35 * sqrt( dy2 + dx * dx ), 255 );
    }
  }

  Backend_ImageMGR_LoadImage( image, image->width, image->height );

  image = ImageMGR_AllocateImage( "*white" );
  ImageMGR_AllocateImageData( image, 2, 2 );
  refctx.image.whiteImageIdx = image->imageIdx;
  for ( int i = 0; i < 4; i++ )
    image->dataRGBA8[ i ] = 0xFFFFFFFF;

  Backend_ImageMGR_LoadImage( image, image->width, image->height );

  image = ImageMGR_AllocateImage( "*gray" );
  ImageMGR_AllocateImageData( image, 2, 2 );
  refctx.image.grayImageIdx = image->imageIdx;
  for ( int i = 0; i < 4; i++ )
    image->dataRGBA8[ i ] = 0xFF7F7F7F;

  Backend_ImageMGR_LoadImage( image, image->width, image->height );

  image = ImageMGR_AllocateImage( "*black" );
  ImageMGR_AllocateImageData( image, 2, 2 );
  refctx.image.blackImageIdx = image->imageIdx;
  for ( int i = 0; i < 4; i++ )
    image->dataRGBA8[ i ] = 0xFF000000;

  Backend_ImageMGR_LoadImage( image, image->width, image->height );
}

void ImageMGR_AllocateImageData( rx_image_t* image, int width, int height ) {
  image->width = width;
  image->height = height;
  image->dataPtr = ENGINE_ALLOC( refctx.memPool, width * height * ( ( image->pixFmt == PIXFMT_RGBA8 ) ? sizeof( pixrgba8_t ) : sizeof( pixrgba4_t ) ) );
}

void ImageMGR_UnloadBrushImages( model_t* model ) {
  dlog( "unloading \"%s\" ", model->name );

  for ( int i = 0; i < model->numtextures; i++ ) {
    texture_t* texture = model->textures[ i ];

    if ( !texture || texture->gl_texturenum == refctx.image.defaultImageIdx )
      continue;

    ImageMGR_FreeImage( texture->gl_texturenum ); // main texture
    ImageMGR_FreeImage( texture->fb_texturenum ); // fullbright texture
  }
}

rx_image_t* ImageMGR_AllocateImage( const char* name ) {
  if ( !name ) return nullptr;

  int freeSlotIdx = ImageMGR_GetFreeSlot( );
  if ( freeSlotIdx == 0 ) {
    dlog( "no free image slots for \"%s\"", name );
    return nullptr;
  }

  rx_image_t* image = &imageMGR.imageList[ freeSlotIdx ];

  ImageMGR_ClearImage( image );

  // also mark the image as used
  image->imageIdx = freeSlotIdx;

  ImageMGR_UpdateImageName( image, name );

  if ( freeSlotIdx >= imageMGR.imageCount ) imageMGR.imageCount = freeSlotIdx + 1;

  return image;
}

void ImageMGR_UpdateImageName( rx_image_t* image, const char* name ) {
  if ( !image || !name ) return;
  strcpy( image->name, name );
  image->nameHash = COM_HashKey( name, MAX_IMAGE_COUNT );
}

void ImageMGR_ClearImage( rx_image_t* image ) {
  if ( !image ) return;

  if ( image->dataRGBA8 ) ENGINE_FREE( image->dataRGBA8 );
  image->dataRGBA8 = nullptr;

  for ( int i = 0; i < MIP_ONLY_COUNT; i++ ) {
    rx_image_t* currMip = image->mips[ i ];
    if ( !currMip ) break; // no mip for this image

    // reached a duplication, break
    if ( i > 0 && currMip == image->mips[ i - 1 ] ) break;

    // recursive call, a mip must not contain a mip of itself in the struct
    ImageMGR_ClearImage( currMip );
  }

  Backend_ImageMGR_FreeImage( image );

  memset( image, 0, sizeof( rx_image_t ) );
}

int ImageMGR_GetImagebyName( const char* name ) {
  if ( !name || !name[ 0 ] ) return 0;

  int nameHash = COM_HashKey( name, MAX_IMAGE_COUNT );

  for ( int i = 1; i < MAX_IMAGE_COUNT; i++ ) {
    rx_image_t* img = &imageMGR.imageList[ i ];

    if ( !Q_stricmp( img->name, name ) )
      return i;
  }

  return 0;
}

int ImageMGR_GetFreeSlot( ) {
  for ( int i = 1; i < MAX_IMAGE_COUNT; i++ ) {
    if ( imageMGR.imageList[ i ].imageIdx == 0 )
      return i;
  }

  return 0;
}

bool ImageMGR_FreeImage( int imageIdx ) {
  if ( imageIdx <= 0 ) return false; // cannot remove sentinel or invalid image index

  // free image data
  ImageMGR_ClearImage( &imageMGR.imageList[ imageIdx ] );

  return true;
}

void ImageMGR_Shutdown( ) {
  for ( int i = 0; i < imageMGR.imageCount; i++ )
    ImageMGR_FreeImage( i );

  engineFuncs->Cmd_RemoveCommand( "texturelist" );
  engineFuncs->Cmd_RemoveCommand( "texturelistmin" );

  memset( imageMGR.imageList, 0, sizeof( imageMGR.imageList ) );

  imageMGR.imageCount = 0;
}

bool ImageMGR_Init( ) {
  if ( !Backend_ImageMGR_Init( ) ) return false;

  memset( imageMGR.imageList, 0, sizeof( imageMGR.imageList ) );
  memset( imageMGR.texUnits, 0, sizeof( imageMGR.texUnits ) );

  rx_image_t* image = &imageMGR.imageList[ 0 ];
  strcpy( image->name, "*sentinel*" );
  image->nameHash = COM_HashKey( image->name, MAX_IMAGE_COUNT );

  imageMGR.imageCount = 1;

  ImageMGR_CreateInternalTextures( );
  ImageMGR_InitAllImageParams( );

  engineFuncs->Cmd_AddCommand( "texturelist", ImageMGR_ImageList, "display loaded textures list" );
  engineFuncs->Cmd_AddCommand( "texturelistmin", ImageMGR_ImageListMinimal, "display loaded textures list minimally" );

  // FIXME: support RGBA4 for soft renderer
  imageMGR.imagePixFmt = ( engineFuncs->Sys_CheckParm( "-rgba4" ) && refctx.backend == BACKEND_GL ) ? PIXFMT_RGBA4 : PIXFMT_RGBA8;

  return true;
}
