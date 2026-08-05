
#pragma once

#include "xash3d_types.h"
#include "const.h"
#include "com_model.h"
#include "com_model.h"
#include "q_client.h"
#include "mod_local.h"

#include "rx_3d_defs.h"

#include "rx_image.h"

#define GL_MAX_TEXTURE_HANDLE_COUNT 8192

#define MAX_LIGHTMAPS 256

enum {
  BACKEND_SOFT = 0,

  // help me check if the gl implementation can also be gles2
  BACKEND_GL,

  // for future expansion
  BACKEND_VK,
  BACKEND_DIRECTX3D9,
  BACKEND_DIRECTX3D11,
  BACKEND_METAL,
  BACKEND_WEBGL,

  BACKEND_COUNT,
};

typedef struct {
  world_static_s* world;
  color24* palette;
  cl_entity_t* viewEntity;
  uint8_t* textureGamma;
  unsigned int* lightGamma;
  unsigned int* screenGamma;
  unsigned int* linearGamma;
  dlight_t* eLights;

} rx_refparms_t;

typedef struct {
  // internal images
  int defaultImageIdx; // aka the famous missing texture
  int whiteImageIdx;
  int grayImageIdx;
  int blackImageIdx;
  int particleImageIdx;
  int cinematicImageIdx;

  // other images
  int solidSkyImageIdx;
  int alphaSkyImageIdx;
  int skyBoxImageIdx[ SKYBOX_SIDES ];
  int lightMapImageIdx[ MAX_LIGHTMAPS ];
  int dynamicLightImageIdx;

} rx_refimage_t;

typedef struct {
  float frameTime;

  // so we dont have to spam engine randomizer function
  unsigned int randomTable[ MOD_FRAMES * MOD_FRAMES ];

  rx_refparms_t params;

  rx_refimage_t image;

  bool clientGameplay;
  bool initialized;

  poolhandle_t memPool;

  int backend;

} rx_refctx_t;
