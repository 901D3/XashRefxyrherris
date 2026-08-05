
#pragma once

typedef struct {
  int startY, endY; // end y is inclusive
  int startXFP, stepXFP;
} rxsw_edge_t;

typedef struct {
  rxsw_edge_t* edgeList;
  int edgeCount;

  int edgeListSize;

} rxsw_edgearena_t;

extern rxsw_edgearena_t edgeArena;

extern bool Soft_Render3D_Edge_ClipEdge( float point1[ 3 ], float point2[ 3 ], float outpoint1[ 3 ], float outpoint2[ 3 ], rx_plane_t planes[], int planeCount );

bool Soft_Render3D_Edge_BuildEdge( float v0[ 3 ], float v1[ 3 ], rxsw_edge_t* outEdge );
bool Soft_Render3D_Edge_BuildEdgeP( float projx0, float projy0, float projx1, float projy1, rxsw_edge_t* outEdge );

int Soft_Render3D_Edge_EmitEdges( float v0[ 3 ], float v1[ 3 ], float v2[ 3 ], rxsw_edge_t* outEdge0, rxsw_edge_t* outEdge1, rxsw_edge_t* outEdge2 );
int Soft_Render3D_Edge_EmitEdgesP( float projx0, float projy0, float projx1, float projy1, float projx2, float projy2, rxsw_edge_t* outEdge0, rxsw_edge_t* outEdge1, rxsw_edge_t* outEdge2 );

void drawedge( rxsw_edge_t* edge );