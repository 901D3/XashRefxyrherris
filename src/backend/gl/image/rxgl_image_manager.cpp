
#include "com_image.h"

#include "rx_cvars.h"
#include "rx_image_manager.h"
#include "rx_memmgr.h"

#include "rxgl_stuff.h"
#include "rxgl_base.h"
#include "rxgl_image_manager.h"

#include "rx_macros.h"

static bool useLinearFilter = true;

BACKEND_EXPORT void GL_ImageMGR_LoadImage( rx_image_t *image, int width, int height ) {
  GLuint *texImageHandle = &image->glTexHandle;

  // generate a handle and save it
  glGenTextures( 1, texImageHandle );
  if ( !( *texImageHandle ) ) return;

  // bind texture for uploading to gpu
  glBindTexture( GL_TEXTURE_2D, *texImageHandle );

  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

  glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

  // upload to gpu
  if ( image->pixFmt == PIXFMT_RGBA8 )
    glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_RGBA,
      width, height,
      0,
      GL_RGBA,
      GL_UNSIGNED_BYTE,
      image->dataPtr );
  else {
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_RGBA4,
      width, height,
      0,
      GL_RGBA,
      GL_UNSIGNED_SHORT_4_4_4_4,
      image->dataPtr );
  }

  // FIXMEFIXMEFIXME: free image data buffer after generate mipmaps 
  // ENGINE_FREE( image->dataRGBA8 );
  // image->dataRGBA8 = nullptr;
}

BACKEND_EXPORT void GL_ImageMGR_FreeImage( rx_image_t *image ) {
  glDeleteTextures( 1, &image->glTexHandle );
  image->glTexHandle = 0;
}

BACKEND_EXPORT bool GL_ImageMGR_Init( ) { return true; }
