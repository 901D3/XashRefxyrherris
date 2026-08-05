
#include "rx_video.h"

#include "rxgl_base.h"

void GL_BuildVertices(float v[6 * 2], float x0, float y0, float x1, float y1) {
  float videoFrameWidthf  = 2.f / videoctx.width;
  float videoFrameHeightf = 2.f / videoctx.height;

  x1 = ((x0 + x1) * videoFrameWidthf) - 1.f;
  y1 = 1.f - ((y0 + y1) * videoFrameHeightf);

  x0 = x0 * videoFrameWidthf - 1.f;
  y0 = 1.f - y0 * videoFrameHeightf;

  // 1---------3
  // |       /
  // |     /
  // |   /
  // | /
  // 2

  //           6
  //         / |
  //       /   |
  //     /     |
  //   /       |
  // 4---------5

  v[0] = x0, v[1] = y0;
  v[2] = x0, v[3] = y1;
  v[4] = x1, v[5] = y0;

  v[6] = x0, v[7] = y1;
  v[8] = x1, v[9] = y1;
  v[10] = x1, v[11] = y0;
}

void GL_BuildTextureVertices(
  float v[6 * 2],
  int imageWidth, int imageHeight,
  float u0, float v0, float u1, float v1) {

  float invImageWidthf  = 1.f / imageWidth;
  float invImageHeightf = 1.f / imageHeight;

  u0 *= invImageWidthf;
  v0 *= invImageHeightf;

  u1 *= invImageWidthf;
  v1 *= invImageHeightf;

  // 1---------3
  // |       /
  // |     /
  // |   /
  // | /
  // 2

  //           6
  //         / |
  //       /   |
  //     /     |
  //   /       |
  // 4---------5

  v[0] = u0, v[1] = v0;
  v[2] = u0, v[3] = v1;
  v[4] = u1, v[5] = v0;

  v[6] = u0, v[7] = v1;
  v[8] = u1, v[9] = v1;
  v[10] = u1, v[11] = v0;
}

void GL_BuildDrawTextureVertices(
  float v[6 * 2 * 2],
  int imageWidth, int imageHeight,
  float scaledWidth, float scaledHeight,
  float x, float y,
  float imageCropLeft, float imageCropTop, float imageCropRight, float imageCropBottom) {

  float invWidthf  = 2.f / static_cast<float>(videoctx.width);
  float invHeightf = 2.f / static_cast<float>(videoctx.height);

  float x0 = x * invWidthf - 1.f;
  float y0 = 1.f - y * invHeightf;

  float x1 = ((x + scaledWidth) * invWidthf) - 1.f;
  float y1 = 1.f - ((y + scaledHeight) * invHeightf);

  float invImageWidthf  = 1.f / static_cast<float>(imageWidth);
  float invImageHeightf = 1.f / static_cast<float>(imageHeight);

  float u0 = imageCropLeft * invImageWidthf;
  float v0 = imageCropTop * invImageHeightf;

  float u1 = imageCropRight * invImageWidthf;
  float v1 = imageCropBottom * invImageHeightf;

  // 1---------3
  // |       /
  // |     /
  // |   /
  // | /
  // 2

  //           6
  //         / |
  //       /   |
  //     /     |
  //   /       |
  // 4---------5

  v[0] = x0, v[1] = y0, v[2] = u0, v[3] = v0;   // TL
  v[4] = x0, v[5] = y1, v[6] = u0, v[7] = v1;   // TR
  v[8] = x1, v[9] = y0, v[10] = u1, v[11] = v0; // BL

  v[12] = x0, v[13] = y1, v[14] = u0, v[15] = v1; // BL
  v[16] = x1, v[17] = y1, v[18] = u1, v[19] = v1; // BR
  v[20] = x1, v[21] = y0, v[22] = u1, v[23] = v0; // TR
}
