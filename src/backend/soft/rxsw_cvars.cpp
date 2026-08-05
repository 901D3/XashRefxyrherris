
#include "cvardef.h"

#include "engine_stuff.h"

#include "rx_base.h"
#include "rx_cvars.h"
#include "rx_macros.h"

#include "rxsw_cvars.h"

M_CVAR_DEFINE_AUTO_CPP( rxsw_maxsurf, "256", FCVAR_GLCONFIG, "" );
M_CVAR_DEFINE_AUTO_CPP( rxsw_maxvert, "512", FCVAR_GLCONFIG, "" );

rxsw_cvarmirrors swCvarMirrors;

BACKEND_EXPORT void Soft_Cvars_SetCvar( convar_t* cvar, const char* string, float value ) { }

BACKEND_EXPORT void Soft_Cvars_SyncCvarMirrors( ) {
  SetCvarMirror( swCvarMirrors.rxsw_maxsurf, rxsw_maxsurf.string );
  SetCvarMirror( swCvarMirrors.rxsw_maxvert, rxsw_maxvert.string );
}

BACKEND_EXPORT void Soft_Cvars_CvarsInit( ) {
  engineFuncs->Cvar_RegisterVariable( &rxsw_maxsurf );
  engineFuncs->Cvar_RegisterVariable( &rxsw_maxvert );
}
