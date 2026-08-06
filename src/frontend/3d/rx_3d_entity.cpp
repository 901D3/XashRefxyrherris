
#include "xash3d_types.h"
#include "cl_entity.h"
#include "entity_types.h"
#include "xash3d_mathlib.h"

#include "rx_base.h"
#include "rx_cvars.h"
#include "rx_3d.h"
#include "rx_3d_entity.h"
#include "rx_3d_surface.h"
#include "rx_image_manager.h"
#include "rx_triapi.h"
#include "rx_studioapi.h"
#include "rx_blend.h"
#include "rx_stats.h"

#include "rx_memmgr.h"
#include "rx_macros.h"

#include "xyrherris-mathlib.h"

entityarena_t entityArena;

GAME_EXPORT void Render3D_Entity_ProcessEntityData_( qboolean allocate, cl_entity_t entityList[], unsigned int entityCount ) { entityArena.entityList = entityList, entityArena.entityCount = entityCount; }

GAME_EXPORT qboolean Render3D_Entity_AddEntity_( cl_entity_s* clEntity, int type ) {
  if ( !cvarMirrors.r_drawentities ) return false;
  if ( clEntity->curstate.effects & EF_NODRAW ) return false;
  if ( !MCR_IsOpaqueEntity( clEntity->curstate.rendermode ) && ( CL_FxBlend( clEntity ) <= 0 ) )
    return true;

  if ( type == ET_BEAM ) {
    if ( entityArena.drawList->beamEntityCount >= MAX_VISIBLE_PACKET ) {
      errlog( "BEAM entity list is full" );
      return false;
    }

    entityArena.drawList->beamEntityList[ entityArena.drawList->beamEntityCount ] = clEntity;
    entityArena.drawList->beamEntityCount++;
  }
  else if ( Render3D_Entity_IsOpaque( clEntity ) ) {
    if ( entityArena.drawList->solidEntityCount >= MAX_VISIBLE_PACKET ) {
      errlog( "SOLID entity list is full" );
      return false;
    }

    entityArena.drawList->solidEntityList[ entityArena.drawList->solidEntityCount ] = clEntity;
    entityArena.drawList->solidEntityCount++;
  }
  else {
    if ( entityArena.drawList->transEntityCount >= MAX_VISIBLE_PACKET ) {
      errlog( "TRANSLUCENT entity list is full" );
      return false;
    }

    entityArena.drawList->transEntityList[ entityArena.drawList->transEntityCount ] = clEntity;
    entityArena.drawList->transEntityCount++;
  }

  return true;
}

GAME_EXPORT void Render3D_Entity_PushScene_( ) {
  if ( ++entityArena.drawListIdx >= MAX_DRAW_STACK ) {
    M_HostError( "draw stack overflow, max draw stack: %i", MAX_DRAW_STACK );
    return;
  }

  entityArena.drawList = &entityArena.drawStack[ entityArena.drawListIdx ];
}

GAME_EXPORT void Render3D_Entity_PopScene_( ) {
  if ( --entityArena.drawListIdx < 0 ) {
    M_HostError( "draw stack underflow" );
    return;
  }

  entityArena.drawList = &entityArena.drawStack[ entityArena.drawListIdx ];
}

void Render3D_Entity_ClearCurrentDrawList( ) {
  entityArena.drawList->solidEntityCount = 0;
  entityArena.drawList->transEntityCount = 0;
  entityArena.drawList->beamEntityCount = 0;
}

int Render3D_Entity_RankRenderModeOrder( int renderMode ) {
  if ( renderMode == kRenderTransTexture )
    return 1;
  else if ( renderMode == kRenderTransAdd )
    return 2;
  else if ( renderMode == kRenderGlow )
    return 3;

  // any other render mode
  return 0;
}

bool Render3D_Entity_IsOpaque( cl_entity_t* clEntity ) {
  if ( Render3D_Entity_GetEntityRenderMode( clEntity ) == kRenderNormal ) {
    int renderFX = clEntity->curstate.renderfx;

    if ( renderFX == kRenderFxNone || renderFX == kRenderFxDeadPlayer || renderFX == kRenderFxLightMultiplier || renderFX == kRenderFxExplode )
      return true;
  }

  return false;
}

int Render3D_Entity_GetEntityRenderMode( cl_entity_t* clEntity ) {
  model_t* entityModel = nullptr;

  cl_entity_t* oldCLEntity = render3D.currEntity;
  render3D.currEntity = clEntity;

  // if the passed entity is a player entity, set up the player entity model
  if ( clEntity->player )
    entityModel = StudioAPI_SetupPlayerModel( clEntity->curstate.number - 1 );

  if ( !entityModel )
    entityModel = clEntity->model;

  render3D.currEntity = oldCLEntity;

  studiohdr_t* header = reinterpret_cast<studiohdr_t*>( engineFuncs->Mod_Extradata( mod_studio, entityModel ) );

  if ( !header ) { // extra data doesnt exist
    // forcing to choose right sorting type
    if (
      ( clEntity->curstate.rendermode == kRenderNormal ) &&
      ( entityModel && entityModel->type == mod_brush ) &&
      ( entityModel->flags & MODEL_TRANSPARENT ) ) return kRenderTransAlpha;

    return clEntity->curstate.rendermode;
  }

  mstudiotexture_t* texture = reinterpret_cast<mstudiotexture_t*>( &reinterpret_cast<byte*>( header )[ header->textureindex ] );

  int opaque = 0, trans = 0;
  for ( int i = 0; i < header->numtextures; i++ ) {
    if ( ( texture[ i ].flags & STUDIO_NF_ADDITIVE ) && !( texture[ i ].flags & STUDIO_NF_CHROME ) )
      trans++;
    else
      opaque++;
  }

  // if model is more additive than opaque
  if ( trans > opaque )
    return kRenderTransAdd;
  return clEntity->curstate.rendermode;
}

void Render3D_Entity_RenderEntities( ) {
#ifdef R_SPEEDS
  int solidBrushCount = 0;
  int solidStudioCount = 0;

  int transBrushCount = 0;
  int transStudioCount = 0;
#endif

  bool dontDrawWorld = render3D.viewpassFlags & RF_DRAW_WORLD;

  if ( !( render3D.viewpassFlags & RF_ONLY_CLIENTDRAW ) ) {
    for ( int i = 0; i < entityArena.drawList->solidEntityCount; i++ ) {
      render3D.currEntity = entityArena.drawList->solidEntityList[ i ];
      render3D.currModel = render3D.currEntity->model;

      if ( !render3D.currModel && render3D.currEntity->player && !dontDrawWorld )
        continue;

      const int modelType = render3D.currModel->type;
      if ( modelType == mod_brush ) {
        Render3D_Entity_DrawBrushEntityModel( );

#ifdef R_SPEEDS
        solidBrushCount++;
#endif
      }

      else if ( modelType == mod_alias )
        ; // unimplemented

      else if ( modelType == mod_studio ) {
        StudioAPI_DrawStudioEntityModel( );

#ifdef R_SPEEDS
        solidStudioCount++;
#endif
      }
    }

    render3D.currEntity = nullptr;
  }

  if ( !( render3D.viewpassFlags & RF_ONLY_CLIENTDRAW ) ) {
    for ( int i = 0; i < entityArena.drawList->transEntityCount; i++ ) {
      render3D.currEntity = entityArena.drawList->transEntityList[ i ];
      render3D.currModel = render3D.currEntity->model;

      if ( !render3D.currModel && render3D.currEntity->player && !dontDrawWorld )
        continue;

      if ( render3D.currEntity->curstate.rendermode != kRenderNormal ) {
        render3D.entityAlpha = CL_FxBlend( render3D.currEntity ) / 255.f;
        if ( render3D.entityAlpha <= 0.f ) continue;
      }
      else
        render3D.entityAlpha = 1.f;

      const int modelType = render3D.currModel->type;
      if ( modelType == mod_brush ) {
        Render3D_Entity_DrawBrushEntityModel( );

#ifdef R_SPEEDS
        transBrushCount++;
#endif
      }

      else if ( modelType == mod_alias )
        ; // unimplemented

      else if ( modelType == mod_studio ) {
        StudioAPI_DrawStudioEntityModel( );

#ifdef R_SPEEDS
        transStudioCount++;
#endif
      }
    }

    render3D.currEntity = nullptr;
  }

#ifdef R_SPEEDS
  if ( cvarMirrors.r_speeds )
    R_SPEEDS_LOG(
      "entities:\n"
      "  solid: %i br, %i st\n"
      "  trans: %i br, %i st",
      solidBrushCount, solidStudioCount,
      transBrushCount, transStudioCount );
#endif
}

int CL_FxBlend( cl_entity_t* e ) {
  int blend = 0;
  float dist;

  float offset = ( ( int )e->index ) * 363.0f; // Use ent index to de-sync these fx

  switch ( e->curstate.renderfx ) {
  case kRenderFxPulseSlowWide:
    blend = e->curstate.renderamt + 0x40 * sin( refClient->time * 2 + offset );
    break;
  case kRenderFxPulseFastWide:
    blend = e->curstate.renderamt + 0x40 * sin( refClient->time * 8 + offset );
    break;
  case kRenderFxPulseSlow:
    blend = e->curstate.renderamt + 0x10 * sin( refClient->time * 2 + offset );
    break;
  case kRenderFxPulseFast:
    blend = e->curstate.renderamt + 0x10 * sin( refClient->time * 8 + offset );
    break;
  case kRenderFxFadeSlow:
    if ( e->curstate.renderamt > 0 )
      e->curstate.renderamt -= 1;
    else
      e->curstate.renderamt = 0;
    blend = e->curstate.renderamt;
    break;
  case kRenderFxFadeFast:
    if ( e->curstate.renderamt > 3 )
      e->curstate.renderamt -= 4;
    else
      e->curstate.renderamt = 0;
    blend = e->curstate.renderamt;
    break;
  case kRenderFxSolidSlow:
    if ( e->curstate.renderamt < 255 )
      e->curstate.renderamt += 1;
    else
      e->curstate.renderamt = 255;
    blend = e->curstate.renderamt;
    break;
  case kRenderFxSolidFast:
    if ( e->curstate.renderamt < 252 )
      e->curstate.renderamt += 4;
    else
      e->curstate.renderamt = 255;
    blend = e->curstate.renderamt;
    break;
  case kRenderFxStrobeSlow:
    blend = 20 * sin( refClient->time * 4 + offset );
    if ( blend < 0 )
      blend = 0;
    else
      blend = e->curstate.renderamt;
    break;
  case kRenderFxStrobeFast:
    blend = 20 * sin( refClient->time * 16 + offset );
    if ( blend < 0 )
      blend = 0;
    else
      blend = e->curstate.renderamt;
    break;
  case kRenderFxStrobeFaster:
    blend = 20 * sin( refClient->time * 36 + offset );
    if ( blend < 0 )
      blend = 0;
    else
      blend = e->curstate.renderamt;
    break;
  case kRenderFxFlickerSlow:
    blend = 20 * ( sin( refClient->time * 2 ) + sin( refClient->time * 17 + offset ) );
    if ( blend < 0 )
      blend = 0;
    else
      blend = e->curstate.renderamt;
    break;
  case kRenderFxFlickerFast:
    blend = 20 * ( sin( refClient->time * 16 ) + sin( refClient->time * 23 + offset ) );
    if ( blend < 0 )
      blend = 0;
    else
      blend = e->curstate.renderamt;
    break;
  case kRenderFxHologram:
  case kRenderFxDistort: {
    vec3_t tmp = Vec3( e->origin );
    VectorSubtract( tmp, render3D.position, tmp );
    dist = DotProduct( tmp, &render3D.angleVector[ ANGLEVEC_FORWARD_IDX ] );

    // turn off distance fade
    if ( e->curstate.renderfx == kRenderFxDistort )
      dist = 1;

    if ( dist <= 0 ) {
      blend = 0;
    }
    else {
      e->curstate.renderamt = 180;
      if ( dist <= 100 )
        blend = e->curstate.renderamt;
      else
        blend = ( int )( ( 1.0f - ( dist - 100 ) * ( 1.0f / 400.0f ) ) * e->curstate.renderamt );
      blend += engineFuncs->COM_RandomLong( -32, 31 );
    }
    break;
  }
  default:
    blend = e->curstate.renderamt;
    break;
  }

  blend = bound( 0, blend, 255 );

  return blend;
}

cl_entity_t* Render3D_Entity_GetParentEntity( cl_entity_t* clEntity, cl_entity_t* clEntityList[], int clEntityCount ) {
  int aiment = clEntity->curstate.aiment;
  for ( int i = 0; i < clEntityCount; i++ ) {
    cl_entity_t* ent = clEntityList[ i ];
    if ( ent->index == aiment ) return ent;
  }

  return nullptr;
}

entity_state_t* Render3D_GetPlayerEntityState( int idx ) {
  if ( !( render3D.viewpassFlags & RF_DRAW_WORLD ) )
    return &render3D.currEntity->curstate;

  return engineFuncs->pfnGetPlayerState( idx );
}

void Render3D_Entity_DrawBrushEntityModel( ) {
  if ( !( render3D.viewpassFlags & RF_DRAW_WORLD ) ) return;

  cl_entity_t* clEntity = render3D.currEntity;
  model_t* entityModel = clEntity->model;
  // force RXSURF flags for every surface emitted from entity's model
  unsigned int surfaceForceFlags = 0;

  // new mins and maxs for translated (and rotated) submodel, for culling only
  float newMins[ 3 ];
  float newMaxs[ 3 ];

  bool rotatedModel = !VectorIsNull( clEntity->angles );

  if ( rotatedModel ) {
    // calculate new mins and maxs
    for ( int i = 0; i < 3; i++ ) {
      float entityOrigin = clEntity->origin[ i ];

      newMins[ i ] = entityOrigin - entityModel->radius;
      newMaxs[ i ] = entityOrigin + entityModel->radius;
    }
  }
  else {
    // no rotation, translate the mins and maxs
    VectorAdd( entityModel->mins, clEntity->origin, newMins );
    VectorAdd( entityModel->maxs, clEntity->origin, newMaxs );
  }

  // check if entity is outside of the frustum, must use the new mins and maxs
  if ( Render3DUtils_CullAABBWrapper( newMins, newMaxs, render3D.frustumPlanes, FRUSTUM_PLANE_COUNT ) ) return;

  MAT3X4ROW_IDENTITY( render3D.modelTransform );

  render3D.modelTransform[ 3 ] = clEntity->origin[ 0 ];
  render3D.modelTransform[ 7 ] = clEntity->origin[ 1 ];
  render3D.modelTransform[ 11 ] = clEntity->origin[ 2 ];

  // write angle vectors to transform matrix if model is rotated
  if ( rotatedModel ) {
    M_Mat3x4Row_AngleVectors(
      clEntity->angles[ PITCH ], clEntity->angles[ YAW ], clEntity->angles[ ROLL ],
      render3D.modelTransform );

    render3D.modelTransfType = TRANSF_AFFINE;
  }
  else {
    render3D.modelTransfType = TRANSF_TRANSLATION;
  }

  VectorSubtract( render3D.position, clEntity->origin, render3D.modelOrigin );

  // mark pvs
  clEntity->visframe = render3D.pvsFrameCount;

  if ( entityModel->flags & MODEL_TRANSPARENT )
    clEntity->curstate.rendermode = kRenderTransAlpha;

  // since brush entities are submodels of world model, we cant do node traverse
  // instead the entity model gave us those metadata for adding surfaces

  int firstSurface = entityModel->firstmodelsurface;
  int lastSurface = firstSurface + entityModel->nummodelsurfaces;

  for ( int i = firstSurface; i < lastSurface; i++ ) {
    msurface_t* mSurface = &entityModel->surfaces[ i ];

    rx_surfaceargs_t surfaceArgs;

    float worldNormal[ 3 ];
    M_Mat3x4Row_MutiplyVectorNoTransl( mSurface->plane->normal, render3D.modelTransform, worldNormal );

    if ( Render3DUtils_FaceCull(
           mSurface->flags & SURF_PLANEBACK,
           render3D.position,
           worldNormal, DotProduct( worldNormal, clEntity->origin ) + mSurface->plane->dist ) ) continue;

    surfaceArgs.mSurface = mSurface;
    surfaceArgs.baseTex = &imageMGR.imageList[ surfaceArgs.mSurface->texinfo->texture->gl_texturenum ];
    surfaceArgs.isFromEntity = true;
    surfaceArgs.blendMode = ToBlendModeSurface( clEntity->curstate.rendermode );
    surfaceArgs.flags = surfaceForceFlags | RXSURF_ENTITY_SUBWORLDMODEL | RXSURF_MODELTRANSFORM;

    if ( clEntity->curstate.rendermode == kRenderTransAlpha )
      surfaceArgs.alphaTestThresh = 0.25f;

    Render3D_Surface_AddSurface( &surfaceArgs );
  }
}

// stub
void Render3D_Entity_DrawAliasEntityModel( ) { }
