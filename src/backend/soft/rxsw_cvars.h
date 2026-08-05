
#pragma once

#include "cvardef.h"

#include "rx_cvars.h"

extern convar_t rxsw_maxsurf;
extern convar_t rxsw_maxvert;

typedef struct {
  CvarMirror<int, 8> rxsw_maxsurf;
  CvarMirror<int, 8> rxsw_maxvert;

} rxsw_cvarmirrors;

extern rxsw_cvarmirrors swCvarMirrors;

extern void Soft_Cvars_SetCvar( convar_t *cvar, const char *string, float value );
extern void Soft_Cvars_SyncCvarMirrors( );
extern void Soft_Cvars_CvarsInit( );
