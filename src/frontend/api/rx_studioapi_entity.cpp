
#include <stdint.h>

#include "com_model.h"
#include "cl_entity.h"
#include "cvardef.h"
#include "r_studioint.h"
#include "studio.h"

#include "engine_stuff.h"

#include "rx_3d.h"
#include "rx_3d_entity.h"
#include "rx_studioapi.h"

#include "rx_logsutils.h"

#include "rx_macros.h"

STUDIOAPI_EXPORT cl_entity_s *StudioAPI_GetCurrentEntity( ) { return render3D.currEntity; }

void StudioAPI_DrawStudioEntityModel( ) {
  if ( render3D.viewpassFlags & RF_DRAW_CUBEMAP ) return;

  cl_entity_t *clEntity = render3D.currEntity;

  StudioAPI_SetupTimings( );

  if ( clEntity->curstate.movetype == MOVETYPE_FOLLOW ) {
    cl_entity_t *parentClEntity = Render3D_Entity_GetParentEntity(
      clEntity,
      entityArena.drawList->solidEntityList,
      entityArena.drawList->solidEntityCount );

    if ( !parentClEntity )
      parentClEntity = Render3D_Entity_GetParentEntity(
        clEntity,
        entityArena.drawList->transEntityList,
        entityArena.drawList->transEntityCount );

    if ( parentClEntity ) {
      render3D.currEntity = parentClEntity;
      StudioAPI_DrawModelInternal( parentClEntity->player, 0 );

      VectorCopy( parentClEntity->curstate.origin, clEntity->curstate.origin );
      VectorCopy( parentClEntity->origin, clEntity->origin );

      render3D.currEntity = clEntity;
    }
  }

  StudioAPI_DrawModelInternal( clEntity->player, STUDIO_RENDER | STUDIO_EVENTS );
}

void StudioAPI_DrawModelInternal( bool isPlayer, int studioFlags ) {
  if ( !( render3D.viewpassFlags & RF_DRAW_WORLD ) ) {
    if ( isPlayer )
      StudioAPI_DrawPlayer( studioFlags, &render3D.currEntity->curstate );
    else
      StudioAPI_DrawModel( studioFlags );
  }
  else {
    if ( isPlayer )
      engineStudioDraw->StudioDrawPlayer( studioFlags, Render3D_GetPlayerEntityState( render3D.currEntity->index - 1 ) );
    else
      engineStudioDraw->StudioDrawModel( studioFlags );
  }
}
