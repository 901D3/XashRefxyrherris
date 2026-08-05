
#pragma once

#include <stdint.h>

#include "com_model.h"
#include "bspfile.h"
#include "cl_entity.h"
#include "ref_params.h"
#include "protocol.h"

#define NEAR_Z_CLIP  0.01f
#define INVZ_EPSILON 0.0001f

// rx_render3dctx_t::angleVector
enum {
  ANGLEVEC_RIGHT = 0,
  ANGLEVEC_UP = 1,
  ANGLEVEC_FORWARD = 2,

  ANGLEVEC_VECTOR_COUNT,

  ANGLEVEC_RIGHT_IDX = ANGLEVEC_RIGHT * 3,
  ANGLEVEC_UP_IDX = ANGLEVEC_UP * 3,
  ANGLEVEC_FORWARD_IDX = ANGLEVEC_FORWARD * 3
};

// rx_render3dctx_t::screenEdges
enum {
  SCREEN_LEFT = 0,
  SCREEN_RIGHT = 1,
  SCREEN_TOP = 2,
  SCREEN_BOTTOM = 3,

  SCREEN_SIDE_COUNT,

  SCREEN_LEFT_IDX = 0 * 3,
  SCREEN_RIGHT_IDX = 1 * 3,
  SCREEN_TOP_IDX = 2 * 3,
  SCREEN_BOTTOM_IDX = 3 * 3,
};

// rx_render3dctx_t::frustumPlanes
enum {
  FRUSTUM_LEFT = 0,
  FRUSTUM_RIGHT = 1,
  FRUSTUM_UP = 2,
  FRUSTUM_DOWN = 3,
  FRUSTUM_NEAR = 4,
  FRUSTUM_FAR = 5,

  FRUSTUM_PLANE_COUNT,
};

// Plane::v
enum {
#ifndef PLANE_X
  PLANE_X = 0,
#endif

#ifndef PLANE_Y
  PLANE_Y,
#endif

#ifndef PLANE_Z
  PLANE_Z,
#endif

  PLANE_DIST = 3,
};

enum {
  TRANSF_IDENTITY = 0,
  TRANSF_TRANSLATION, // translation only
  TRANSF_AFFINE,      // apply the whole matrix
};

typedef struct {
  float normal[ 3 ];
  float dist;
} rx_plane_t;

typedef struct {
  float position[ 3 ];
  float projx, projy, invz;
  float u, v;
} rx_svertex_t;

typedef struct {

  // general PODs
  struct {
    // time between current frame and previous frame
    float frameTime;

    // PVS frame count
    unsigned int pvsFrameCount;

    // R_FatPVS will give us PVS data so we save data to this
    uint8_t fatPVSData[ ( MAX_MAP_LEAFS + 7 ) >> 3 ];

    int leafKeys[ MAX_MAP_LEAFS ];

    // real frame counts, increment every render 3D frame
    unsigned int frameCount;

    // only increment in render 3D SCENE
    unsigned int sceneFrameCount;

    int viewport[ 4 ]; // x, y, width, height

    int currentSurfaceKey;

    int mapSampleBits;
    int mapSampleSize;

    unsigned int params;

    unsigned int viewpassFlags;

    bool processFrame;
    bool customRendering;
    bool unloadMap;
    bool resetVis;
    bool loadedGameplay;
    bool flipViewModel;
  };

  /*
  ## Model/Entity
  */
  struct {
    // current model to draw
    model_t* currModel;
    model_t* worldModel;

    cl_entity_t* entityList;
    int entityCount;

    // current entity to draw
    cl_entity_t* currEntity;

    float entityAlpha;

    // optimizations

    // when adding a surface with msurface_t, the function will use this matrix to transform the surface
    // default to identity
    float modelTransform[ 12 ];

    // for bmodels and stuff
    // default to camera position
    float modelOrigin[ 3 ];

    int modelTransfType; // TRANSF_*
  };

  /*
  ## Camera
  */
  struct {
    // assume pitch and yaw is all 0 deg
    // x++ => move forward
    // y++ => move left
    // z++ => go up
    float position[ 3 ];

    // in degree
    // turn right(CW) => yaw--
    // turn left(CCW) => yaw++
    // look up => pitch increases
    float viewAngles[ 3 ]; // pitch, yaw, roll(unused)

    // center position of projection, default to center of the screen
    float projCenterX;
    float projCenterY;

    // projection stretching
    // belongs to screen mapping
    float projScaleX;
    float projScaleY;

    float invProjScaleX;
    float invProjScaleY;

    // directions. each one is 3D direction vector
    // we will build frustum corners, planes and projection directions from these later
    float angleVector[ ANGLEVEC_VECTOR_COUNT * 3 ];

    // FOV values in degree, engine passed those values(ref_viewpass_s/ref_viewpass_t)
    // it defined how open the frustum is
    float fovX;
    float fovY;

    float cotFOVX;
    float cotFOVY;

    // camera aspect ratio, width / height
    float aspect;

    // left, right, up, down. near, far
    // anything that is outside of camera frustum is culled
    rx_plane_t frustumPlanes[ 6 ];

    // left, right, top, bottom
    float screenEdges[ 4 * 3 ];

    // MVP stuff
    float view[ 4 * 4 ];
    float projection[ 4 * 4 ];
    float mvp[ 4 * 4 ];
  };

  float fogDensity;
  float fogStart;
  float fogEnd;
  float fogColorf[ 4 ];
  bool fogEnabled;
  bool fogSkyboxImageIdx;
  bool fogCustom;

} rx_render3dctx_t;
