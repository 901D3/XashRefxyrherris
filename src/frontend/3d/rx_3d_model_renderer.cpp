
#include "const.h"

extern "C" {
#include "xash3d_mathlib.h"
}

#include "engine_stuff.h"

#include "rx_cvars.h"
#include "rx_3d.h"
#include "rx_3d_surface.h"
#include "rx_image_manager.h"
#include "rx_blend.h"
#include "rx_triapi.h"
#include "rx_stats.h"

#include "rx_logsutils.h"
#include "xyrherris-mathlib.h"

#include "rx_memmgr.h"

#ifdef R_SPEEDS
  #include <chrono>
#endif

void Render3D_MarkVisibleLeafs( ) {
#ifdef R_SPEEDS
  auto t0 = std::chrono::high_resolution_clock::now( );
#endif

  model_t* model = render3D.currModel;
  const int leafNodeCount = model->numleafs;

  // preshift by 1
  mleaf_t* mleafs = &model->leafs[ 1 ];

  // update pvsFrameCount before visframe assigns, so visframe later wont be behind the pvsFrameCount
  render3D.pvsFrameCount++;

  // get pvs data
  engineFuncs->R_FatPVS(
    render3D.position,
    cvarMirrors.r_pvs_radius,
    render3D.fatPVSData,
    false,
    false );

  const uint8_t* fatPVSData = render3D.fatPVSData;

#ifdef R_SPEEDS
  int visLeafCount = 0;
#endif

  // the output pvs data have leafCount bits, every bits is a bool that tell us if leaf is visible or not
  // 1 for visible and vice versa

  // go through every leaf nodes
  for ( int i = 0; i < leafNodeCount; i++ ) {
    const int byteIdx = i >> 3;

    const int bits = fatPVSData[ byteIdx ];
    if ( !bits ) continue;

    const int mask = 1 << ( i & 7 );
    if ( !( bits & mask ) ) continue;

#ifdef R_SPEEDS
    visLeafCount++;
#endif

    mnode_t* modelNode = ( mnode_t* )( &mleafs[ i ] ); // skip leafs[0] since its a dummy leaf

    // mark the leaf node and all of its parent as render later, by setting visframe to pvsFrameCount
    while ( modelNode ) {
      // skip marked visible nodes
      if ( modelNode->visframe == render3D.pvsFrameCount )
        break;

      // assign visframe to nodes that is visible and visframe isnt synced
      modelNode->visframe = render3D.pvsFrameCount;

      // go to the parent of the current node and mark as visible
      // so later the targeted node is rendered
      // because bsp node traversal starts at root, we will go down and check if the node is visible
      // if a node is visible but its parent isnt then the traversal skips it which sucks
      modelNode = modelNode->parent;
    }
  }

#ifdef R_SPEEDS
  if ( cvarMirrors.r_speeds )
    R_SPEEDS_LOG( "visleafs: %i, mark leafs: %f ms", visLeafCount, ( ( std::chrono::duration<float> )( std::chrono::high_resolution_clock::now( ) - t0 ) ).count( ) * 1000.f );
#endif
}

void Render3D_MarkVisibleSurfaceForLeaf( mleaf_t* leafNode ) {
  const unsigned int pvsFrameCount = render3D.pvsFrameCount;

  if ( leafNode->efrags )
    engineFuncs->R_StoreEfrags( &leafNode->efrags, pvsFrameCount );

  const int surfaceCount = leafNode->nummarksurfaces;
  if ( surfaceCount <= 0 )
    return;

  // assign every surfaces' visframe to pvsFrameCount
  msurface_t** surfacesPtr = leafNode->firstmarksurface;
  for ( int i = 0; i < surfaceCount; i++ )
    surfacesPtr[ i ]->visframe = pvsFrameCount;
}

void Render3D_ModelNodeTraverse( mnode_t* modelNode ) {
  if ( modelNode->contents == CONTENTS_SOLID ) // the current node is solid, we skip it
    return;

  if ( modelNode->visframe != render3D.pvsFrameCount ) // current node is not marked visible, return
    return;

  // if modelNode's content is a leaf node, draw stuff
  if ( modelNode->contents < 0 ) {
    // cast to mleaf_t since the current modelNode is a leaf node
    mleaf_t* leafNode = ( mleaf_t* )( modelNode );

    Render3D_MarkVisibleSurfaceForLeaf( leafNode );

    M_LeafKey( leafNode ) = render3D.currentSurfaceKey;
    render3D.currentSurfaceKey++;
    return;
  }

  // if modelNode's content is a branch(have child nodes), traverse through all of the child nodes
  model_t* currentModel = render3D.currModel;

  mplane_t* nodePlane = modelNode->plane;
  int planeType = nodePlane->type;

  // know if the camera's position is behind, in front or on the plane
  float dot;
  if ( planeType == PLANE_X || planeType == PLANE_Y || planeType == PLANE_Z ) // forward/back
    dot = render3D.position[ planeType ] - nodePlane->dist;

  else // default to dot product
    dot = DotProduct( render3D.position, nodePlane->normal ) - nodePlane->dist;

  int childrenPickSide = dot < 0;

  // a branched node can only have max children of 2
  mnode_t* children[ 2 ];

  // assign childrens to the temp children array, children nodes is taken from modelNode
  node_children( children, modelNode, currentModel );

  // traverse down the front side first
  Render3D_ModelNodeTraverse( children[ childrenPickSide ] );

  // add the edges and surfaces for the front side node
  // guaranteed current node and node's surfaces is marked visible

  int nodeSurfaceCount = node_numsurfaces( modelNode, currentModel );
  if ( nodeSurfaceCount > 0 ) {
    int firstSurfaceIdx = node_firstsurface( modelNode, currentModel );
    int lastSurfaceIdx = firstSurfaceIdx + nodeSurfaceCount;

    for ( int i = firstSurfaceIdx; i < lastSurfaceIdx; i++ ) {
      msurface_t* mSurface = &currentModel->surfaces[ i ];

      if ( mSurface->visframe != render3D.pvsFrameCount ) continue;

      bool flagCheck = mSurface->flags & SURF_PLANEBACK;
      if ( ( !flagCheck && dot < -BACKFACE_EPSILON ) || ( flagCheck && dot > BACKFACE_EPSILON ) ) continue;

      if ( Render3DUtils_CullAABBWrapper(
             mSurface->info->mins, mSurface->info->maxs,
             render3D.frustumPlanes, FRUSTUM_PLANE_COUNT ) ) continue;

      rx_surfaceargs_t rxSurface;
      rxSurface.baseTex = &imageMGR.imageList[ mSurface->texinfo->texture->gl_texturenum ];
      rxSurface.mSurface = mSurface;
      rxSurface.flags = RXSURF_WORLDMODEL |
        RXSURF_UVCACHED; // world model uvs is cached so add this flag

      Render3D_Surface_AddSurface( &rxSurface );
    }

    render3D.currentSurfaceKey++;
  }

  // traverse down the back side, we dont draw the back side node
  Render3D_ModelNodeTraverse( children[ !childrenPickSide ] );
}

void Render3D_RenderModel( ) { Render3D_ModelNodeTraverse( render3D.currModel->nodes ); }

void Render3D_RenderWorldModel( ) {
#ifdef R_SPEEDS
  auto t0 = std::chrono::high_resolution_clock::now( );
#endif

  MAT3X4ROW_IDENTITY( render3D.modelTransform );

  render3D.currModel = render3D.worldModel;

  // mark all PVS leafs for bsp node traversal
  Render3D_MarkVisibleLeafs( );

  Render3D_ModelNodeTraverse( render3D.currModel->nodes );

#ifdef R_SPEEDS
  if ( cvarMirrors.r_speeds )
    R_SPEEDS_LOG( "bsp node traversal: %f ms", ( ( std::chrono::duration<float> )( std::chrono::high_resolution_clock::now( ) - t0 ) ).count( ) * 1000.f );
#endif
}
