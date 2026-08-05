
#include <stdint.h>
#include <math.h>

#include "crclib.h"

#include "rx_base.h"
#include "rx_video.h"
#include "rx_image.h"
#include "rx_image_manager.h"
#include "rx_triapi.h"
#include "rx_macros.h"
#include "rx_memmgr.h"
#include "rx_logsutils.h"

#include "rxgl_stuff.h"
#include "rxgl_cvars.h"
#include "rxgl_draw2d.h"

#include "xyrherris-mathlib.h"

#ifdef R_SPEEDS
int imgDrawCmdCount;
int imgDrawCmdFlushCount;
#endif

#define RECT_VERTEXQUAD_COUNT 24

typedef struct {
  GLuint programId;

  struct {
    GLint position;
  } attr;

  struct {
    GLint color;
    GLint blendMode;
  } unif;

} rxgl_prog_fillrect_t;

typedef struct {
  GLuint programId;

  struct {
    GLint position;
    GLint textureCoord;
  } attr;

  struct {
    GLint textureUnit;
    GLint washColor;
    GLint blendMode;
  } unif;

} rxgl_prog_drawimage_t;

// simple struct for adding image and metadata for drawing
// must store wash color, blend mode and texture handle for a image draw request
typedef struct {
  // those are passed to glsl as uniform so we dont save it to the pool
  float washColorf[ 4 ];
  int blendMode;
  GLuint texHandle;
} rxgl_imagedrawcmds_t;

typedef struct {
  // we MUST flush before draw anything else that affects z index and doesnt belong to this arena
  rxgl_imagedrawcmds_t* imgDrawCmds;
  int imgDrawCmdCount;

  // contiguous vertex quads for every images that exists in the list
  // we must save the vertex quad idx when adding an image to list
  float* vertexQuadPool;
  int vertexQuadCount;

  // we can control this with a cvar
  int imgDrawCmdSize;

  // when flushing, we must use an increment loop because by design, draw 2D stuff always draw to the top layer
  // if we use decrement loop otherwise, the z index is inverted

  // upload the vertex quad pool to this vbo
  GLuint draw2DVBO;

} rxgl_draw2darena;

static rxgl_draw2darena draw2DArena;

static rxgl_prog_fillrect_t fillRectProg;
static rxgl_prog_drawimage_t drawImageProg;

BACKEND_EXPORT void GL_Draw2D_FillRect( float x, float y, float width, float height ) {
  // make sure the queue is flushed before fill rect
  GL_Draw2D_DrawImageFlush( );
  GL_Draw2D_ArenaInit( );

  // disable depth test cuz we are drawing on top of everything and dont want z bufer to mess with it
  GL_SetState( GL_DEPTH_TEST, false );
  GL_SetState( GL_CULL_FACE, false );

  GL_UseProgram( fillRectProg.programId );
  GL_BindBuffer( GL_ARRAY_BUFFER, draw2DArena.draw2DVBO );

  float vertices[ 12 ];
  GL_BuildVertices( vertices, x, y, width, height );
  glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof( vertices ), vertices );

  glEnableVertexAttribArray( fillRectProg.attr.position );
  glVertexAttribPointer(
    fillRectProg.attr.position,
    2,
    GL_FLOAT,
    GL_FALSE,
    2 * SIZEOF_FLOAT,
    reinterpret_cast<void*>( 0 ) );

  glUniform4fv( fillRectProg.unif.color, 1, triapi.washColorf );
  glUniform1i( fillRectProg.unif.blendMode, triapi.blendMode );

  GL_TriAPI_SetBlendMode( triapi.blendMode );

  glDrawArrays( GL_TRIANGLES, 0, 6 );
}

BACKEND_EXPORT void GL_Draw2D_DrawStretchImage(
  int x, int y, int scaledWidth, int scaledHeight,
  int imageCropLeft, int imageCropTop,
  int imageCropRight, int imageCropBottom,
  rx_image_t* image ) {

  if ( !image->glTexHandle ) return;

  if ( draw2DArena.imgDrawCmdCount >= draw2DArena.imgDrawCmdSize || draw2DArena.vertexQuadCount >= draw2DArena.imgDrawCmdSize ) {
    GL_Draw2D_DrawImageFlush( );
    GL_Draw2D_ArenaInit( );
  }

  GL_BuildDrawTextureVertices(
    &draw2DArena.vertexQuadPool[ draw2DArena.vertexQuadCount++ * RECT_VERTEXQUAD_COUNT ],
    image->width, image->height,
    scaledWidth, scaledHeight,
    x, y,
    imageCropLeft, imageCropTop,
    imageCropRight, imageCropBottom );

  rxgl_imagedrawcmds_t* glImgDrawCmds = &draw2DArena.imgDrawCmds[ draw2DArena.imgDrawCmdCount++ ];

  Vector4Copy( triapi.washColorf, glImgDrawCmds->washColorf );
  glImgDrawCmds->blendMode = triapi.blendMode;
  glImgDrawCmds->texHandle = image->glTexHandle;

#ifdef R_SPEEDS
  imgDrawCmdCount++;
#endif
}

void GL_Draw2D_DrawImageFlush( ) {
  GL_SetState( GL_DEPTH_TEST, false );

  GL_UseProgram( drawImageProg.programId );
  GL_BindBuffer( GL_ARRAY_BUFFER, draw2DArena.draw2DVBO );

  const GLsizeiptr usedBytes = draw2DArena.vertexQuadCount * RECT_VERTEXQUAD_COUNT * SIZEOF_FLOAT;

  glBufferData( GL_ARRAY_BUFFER, draw2DArena.imgDrawCmdSize * RECT_VERTEXQUAD_COUNT * SIZEOF_FLOAT, nullptr, GL_DYNAMIC_DRAW );

  glBufferSubData(
    GL_ARRAY_BUFFER,
    0,
    draw2DArena.vertexQuadCount * RECT_VERTEXQUAD_COUNT * SIZEOF_FLOAT,
    draw2DArena.vertexQuadPool );

  // set texture
  // we use texture unit 0
  glActiveTexture( GL_TEXTURE0 );
  // we will active texture unit 0 in the draw image
  glUniform1i( drawImageProg.unif.textureUnit, 0 );

  // setting vertex pointers
  // the vertices quad is interleaved bwteen screen space and clip space(the image cropping things)
  // so when go through the vertices quad, we take the first 2 floats, increment by the stride(4 because x,y,u,v)
  // the first x,y position in a stride is 0
  glEnableVertexAttribArray( drawImageProg.attr.position );
  glVertexAttribPointer(
    drawImageProg.attr.position,
    2,
    GL_FLOAT,
    GL_FALSE,
    4 * SIZEOF_FLOAT,
    reinterpret_cast<void*>( 0 ) );

  // this one, we take the last 2 floats
  // the first u,v position in a stride is 2
  glEnableVertexAttribArray( drawImageProg.attr.textureCoord );
  glVertexAttribPointer(
    drawImageProg.attr.textureCoord,
    2,
    GL_FLOAT,
    GL_FALSE,
    4 * SIZEOF_FLOAT,
    reinterpret_cast<void*>( 2 * SIZEOF_FLOAT ) );

  int currBlendMode = -1;
  int currTexHandle = -1;

  for ( int i = 0; i < draw2DArena.imgDrawCmdCount; i++ ) {
    rxgl_imagedrawcmds_t* glImgDrawCmd = &draw2DArena.imgDrawCmds[ i ];

    glUniform4fv( drawImageProg.unif.washColor, 1, glImgDrawCmd->washColorf );

    if ( currBlendMode != glImgDrawCmd->blendMode ) {
      currBlendMode = glImgDrawCmd->blendMode;

      glUniform1i( drawImageProg.unif.blendMode, currBlendMode );

      GL_TriAPI_SetBlendMode( currBlendMode );
    }

    if ( currTexHandle != glImgDrawCmd->texHandle ) {
      currTexHandle = glImgDrawCmd->texHandle;

      // bind the handler so its active, texture unit 0 is bound to tex handle
      glBindTexture( GL_TEXTURE_2D, currTexHandle );
    }

    glDrawArrays( GL_TRIANGLES, i * 6, 6 );
  }

#ifdef R_SPEEDS
  imgDrawCmdFlushCount++;
#endif
}

void GL_Draw2D_ArenaInit( ) {
  if ( draw2DArena.imgDrawCmdSize != glCvarMirrors.rxgl_imgdrawcmdssize.value ) {
    if ( glCvarMirrors.rxgl_imgdrawcmdssize.value < 1 )
      warnlog( "draw image list size %i is less than 1", glCvarMirrors.rxgl_imgdrawcmdssize.value );

    draw2DArena.imgDrawCmdSize = Q_max( glCvarMirrors.rxgl_imgdrawcmdssize.value, 1 );

    if ( draw2DArena.imgDrawCmds ) ENGINE_FREE( draw2DArena.imgDrawCmds );
    if ( draw2DArena.vertexQuadPool ) ENGINE_FREE( draw2DArena.vertexQuadPool );

    draw2DArena.imgDrawCmds = reinterpret_cast<rxgl_imagedrawcmds_t*>( ENGINE_ALLOC( refctx.memPool, draw2DArena.imgDrawCmdSize * sizeof( rxgl_imagedrawcmds_t ) ) );
    draw2DArena.vertexQuadPool = reinterpret_cast<float*>( ENGINE_ALLOC( refctx.memPool, draw2DArena.imgDrawCmdSize * RECT_VERTEXQUAD_COUNT * SIZEOF_FLOAT ) );

    GL_BindBuffer( GL_ARRAY_BUFFER, draw2DArena.draw2DVBO );
    glBufferData(
      GL_ARRAY_BUFFER,
      draw2DArena.imgDrawCmdSize * RECT_VERTEXQUAD_COUNT * SIZEOF_FLOAT,
      nullptr,
      GL_DYNAMIC_DRAW );
  }

  draw2DArena.imgDrawCmdCount = 0;
  draw2DArena.vertexQuadCount = 0;
}

void GL_Draw2D_ArenaShutdown( ) {
  if ( draw2DArena.imgDrawCmds ) ENGINE_FREE( draw2DArena.imgDrawCmds );
  if ( draw2DArena.vertexQuadPool ) ENGINE_FREE( draw2DArena.vertexQuadPool );

  draw2DArena.imgDrawCmds = nullptr;
  draw2DArena.vertexQuadPool = nullptr;

  draw2DArena.imgDrawCmdCount = 0;
  draw2DArena.vertexQuadCount = 0;
}

bool GL_Draw2D_FillRectProgramInit( ) {
  const char* fillRectVert =
#include "rxgl_s_draw2d_fillrect.vert.h"
    ;
  const char* fillRectFrag =
#include "rxgl_s_draw2d_fillrect.frag.h"
    ;

  fillRectProg.programId = M_CreateShaderProg( "fillRect", fillRectVert, fillRectFrag );
  if ( !fillRectProg.programId ) return false;

  M_GLProgSetAttr( &fillRectProg, A_POSITION_VAR, A_POSITION );

  M_GLProgSetUnif( &fillRectProg, U_COLOR_VAR, U_COLOR );
  M_GLProgSetUnif( &fillRectProg, U_BLENDMODE_VAR, U_BLENDMODE );

  return true;
}

bool GL_Draw2D_DrawImageProgramInit( ) {
  const char* drawImageVert =
#include "rxgl_s_draw2d_drawimage.vert.h"
    ;
  const char* drawImageFrag =
#include "rxgl_s_draw2d_drawimage.frag.h"
    ;

  drawImageProg.programId = M_CreateShaderProg( "drawImage", drawImageVert, drawImageFrag );
  if ( !drawImageProg.programId ) return false;

  M_GLProgSetAttr( &drawImageProg, A_POSITION_VAR, A_POSITION );
  M_GLProgSetAttr( &drawImageProg, A_TEXTURECOORD_VAR, A_TEXTURECOORD );

  M_GLProgSetUnif( &drawImageProg, U_TEXTUREUNIT_VAR, U_TEXTUREUNIT );
  M_GLProgSetUnif( &drawImageProg, U_WASHCOLOR_VAR, U_WASHCOLOR );
  M_GLProgSetUnif( &drawImageProg, U_BLENDMODE_VAR, U_BLENDMODE );

  return true;
}

bool GL_Draw2D_BaseInit( ) {
  if ( !GL_Draw2D_DrawImageProgramInit( ) ) return false;
  if ( !GL_Draw2D_FillRectProgramInit( ) ) return false;

  glGenBuffers( 1, &draw2DArena.draw2DVBO );

  return true;
}
