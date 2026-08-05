
#pragma once

#include "xash3d_types.h"
#include "const.h"
#include "cl_entity.h"

// menu and gameplay
#define MAX_DRAW_STACK 2

#define MCR_IsOpaqueEntity( rendermode ) ( rendermode == kRenderNormal )

typedef struct {
  cl_entity_t *solidEntityList[ MAX_VISIBLE_PACKET ];
  int solidEntityCount;

  cl_entity_t *transEntityList[ MAX_VISIBLE_PACKET ];
  int transEntityCount;

  // ET_BEAM
  cl_entity_t *beamEntityList[ MAX_VISIBLE_PACKET ];
  int beamEntityCount;

} entitydrawlist_t;

typedef struct {
  // engine will pass every entities of the map to this
  cl_entity_t *entityList;
  int entityCount;

  entitydrawlist_t drawStack[ MAX_DRAW_STACK ];
  int drawListIdx;          // track which draw stack we are at
  entitydrawlist_t *drawList; // targeted draw list, points to drawStack[i]

} entityarena_t;

extern entityarena_t entityArena;

extern void Render3D_Entity_ProcessEntityData_( qboolean allocate, cl_entity_t entityList[], unsigned int entityCount );
extern qboolean Render3D_Entity_AddEntity_( cl_entity_s *clEntity, int type );

extern void Render3D_Entity_PushScene_( );
extern void Render3D_Entity_PopScene_( );

extern void Render3D_Entity_ClearCurrentDrawList( );

extern int Render3D_Entity_RankRenderModeOrder( int renderMode );

extern bool Render3D_Entity_IsOpaque( cl_entity_t *clEntity );
extern int Render3D_Entity_GetEntityRenderMode( cl_entity_t *clEntity );
extern void Render3D_Entity_SetBlendMode( cl_entity_t *clEntity );

extern void Render3D_Entity_RenderEntities( );

extern void Render3D_RotateForEntity( cl_entity_t *clEntity );

extern int CL_FxBlend( cl_entity_t *e );

extern cl_entity_t *Render3D_Entity_GetParentEntity( cl_entity_t *clEntity, cl_entity_t *clEntityList[], int clEntityCount );

extern entity_state_t *Render3D_GetPlayerEntityState( int idx );

extern void Render3D_Entity_DrawBrushEntityModel( );
extern void Render3D_Entity_DrawAliasEntityModel( );
