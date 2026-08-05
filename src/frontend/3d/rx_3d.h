
#pragma once

#include "ref_params.h"

#include "rx_3d_defs.h"

#include "rx_macros.h"

#include "xyrherris-mathlib.h"

#define WORLD_MODEL ( refClient->models[ 1 ] )

#define M_CalcGradient( origin, xStep, yStep, x, y ) ( ( origin ) + ( ( xStep ) * ( x ) ) + ( ( yStep ) * ( y ) ) )

/*
## 3D.cpp
*/
#define VIEWPASS_DRAW_WORLD    ( render3D.viewpassFlags & RF_DRAW_WORLD )
#define VIEWPASS_DRAW_CUBEMAP  ( render3D.viewpassFlags & RF_DRAW_CUBEMAP )
#define VIEWPASS_DRAW_OVERVIEW ( render3D.viewpassFlags & RF_DRAW_OVERVIEW )
#define VIEWPASS_DRAW_ONLYCL   ( render3D.viewpassFlags & RF_ONLY_CLIENTDRAW )

#define VIEWPASS_IS_NORMAL_PASS ( !( render3D.params & 1 ) )

extern rx_render3dctx_t render3D;

extern void Render3D_Render3DSCENE_( );
extern void Render3D_ClearScene_( );
extern void Render3D_SyncViewpassParams( const ref_viewpass_s* engineViewPass );
extern void Render3D_OnViewChanged( );

static inline void Render3D_RotatePointToCam( float point[ 3 ], float out[ 3 ] ) { M_RotatePointfv( point, &render3D.angleVector[ ANGLEVEC_FORWARD_IDX ], &render3D.angleVector[ ANGLEVEC_RIGHT_IDX ], &render3D.angleVector[ ANGLEVEC_UP_IDX ], out ); }

extern float Render3D_WorldToScreen( float worldPoint[ 3 ], float originPosition[ 3 ], float* outScreenX, float* outScreenY );
extern void Render3D_TransformPoint( float* point, float* out );
extern void Render3D_TransformAABB( float mins[ 3 ], float maxs[ 3 ], float mat[ 12 ], float outMins[ 3 ], float outMaxs[ 3 ] );
extern void Render3D_BuildMVP( );
extern void Render3D_Shutdown( );
extern void Render3D_InitScene( );
extern void Render3D_OnMapLoad( );

/*
## ModelRenderer.cpp
*/
#define M_GetModelLeafIdx( leaf ) ( leaf - WORLD_MODEL->leafs )
#define M_LeafKey( leaf )         ( render3D.leafKeys[ leaf - WORLD_MODEL->leafs ] )

extern void Render3D_MarkVisibleLeafs( );
extern void Render3D_MarkVisibleSurfaceForLeaf( mleaf_t* modelLeafNode );
extern void Render3D_ModelNodeTraverse( mnode_t* modelNode );
extern void Render3D_RenderModel( );
extern void Render3D_RenderWorldModel( );

/*
## Camera.cpp
*/
extern void Render3D_BuildScreenEdges( );
extern void Render3D_BuildFrustumPlanes( );
extern void Render3D_CvarSyncNearFarZ( );

/*
## 3DUtils.cpp
*/
enum {
  FULLY_OUTSIDE = 0,
  PARTIALLY_OUTSIDE,
  FULLY_INSIDE
};

extern int Render3DUtils_SutherlandHodgman( float inVertices[], int inVertexCount, float outVertices[], rx_plane_t* plane );
extern int Render3DUtils_ClipPolygon( float inVertices[], int inVertexCount, float outVertices[], int outVertexCount, rx_plane_t clipPlanes[], int clipPlaneCount );
extern int Render3DUtils_ClipSPolygon( rx_svertex_t inSVertices[], int svertexCount, rx_svertex_t outSVertices[], int outSVertexCount, rx_plane_t planes[], int planeCount );

static inline void Render3DUtils_LerpSVertex( rx_svertex_t* svertex, const rx_svertex_t* a, const rx_svertex_t* b, float t ) {
  svertex->position[ 0 ] = a->position[ 0 ] + ( b->position[ 0 ] - a->position[ 0 ] ) * t;
  svertex->position[ 1 ] = a->position[ 1 ] + ( b->position[ 1 ] - a->position[ 1 ] ) * t;
  svertex->position[ 2 ] = a->position[ 2 ] + ( b->position[ 2 ] - a->position[ 2 ] ) * t;

  svertex->u = a->u + ( b->u - a->u ) * t;
  svertex->v = a->v + ( b->v - a->v ) * t;
}

extern bool Render3DUtils_LiangBarsky( float p, float q, float* t0, float* t1 );
extern bool Render3DUtils_LiangBarskyRect( float* x0, float* y0, float* x1, float* y1, float xMin, float yMin, float xMax, float yMax );
extern bool Render3DUtils_CullAABB( float center[ 3 ], float extents[ 3 ], rx_plane_t* plane );
extern int Render3DUtils_IsFullyInside( float vertices[], int vertexCount, rx_plane_t planes[], int planeCount );
extern bool Render3DUtils_CullAABBWrapper( float mins[ 3 ], float maxs[ 3 ], rx_plane_t planes[], int planeCount );
extern bool Render3DUtils_FaceCull( bool negate, float* model, float* normal, float dist );

extern void Render3DUtils_CatmullClark3D( float vertices[], int vertexCount, float outVertices[] );
extern void Render3DUtils_SplitSingleQuad( float vertices[], float outVertices[] );
extern void Render3DUtils_SubdivideQuadWrapper( float vertices[], int count, float outVertices[] );
