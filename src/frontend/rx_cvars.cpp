
#include <string.h>
#include <stdlib.h>

#include "cvardef.h"

#include "engine_stuff.h"

#include "rx_backend.h"

#include "rx_base.h"
#include "rx_cvars.h"
#include "rx_video.h"
#include "rx_3d.h"
#include "rx_image_manager.h"

#include "xyrherris-mathlib.h"

rx_cvarmirrors_t cvarMirrors;

M_CVAR_DEFINE_AUTO_CPP( rx_drawimageidx, "0", 0, "" );

M_CVAR_DEFINE_AUTO_CPP( rx_camnearz, "1.0", FCVAR_ARCHIVE, "" );
M_CVAR_DEFINE_AUTO_CPP( rx_camfarz, "1000000.0", FCVAR_ARCHIVE, "" );

// mcr_CVAR_DEFINE_AUTO_CPP(rxyr_triapi_name, "0", 0, ""); mcr_CVAR_DEFINE_AUTO_CPP(rxyr_triapi_vert1_x, "0", 0, ""); mcr_CVAR_DEFINE_AUTO_CPP(rxyr_triapi_vert1_y, "0", 0, ""); mcr_CVAR_DEFINE_AUTO_CPP(rxyr_triapi_vert1_z, "0", 0, ""); mcr_CVAR_DEFINE_AUTO_CPP(rxyr_triapi_vert2_x, "0", 0, ""); mcr_CVAR_DEFINE_AUTO_CPP(rxyr_triapi_vert2_y, "0", 0, ""); mcr_CVAR_DEFINE_AUTO_CPP(rxyr_triapi_vert2_z, "0", 0, ""); mcr_CVAR_DEFINE_AUTO_CPP(rxyr_triapi_vert3_x, "0", 0, ""); mcr_CVAR_DEFINE_AUTO_CPP(rxyr_triapi_vert3_y, "0", 0, ""); mcr_CVAR_DEFINE_AUTO_CPP(rxyr_triapi_vert3_z, "0", 0, ""); mcr_CVAR_DEFINE_AUTO_CPP(rxyr_triapi_r, "255", 0, ""); mcr_CVAR_DEFINE_AUTO_CPP(rxyr_triapi_g, "255", 0, ""); mcr_CVAR_DEFINE_AUTO_CPP(rxyr_triapi_b, "255", 0, ""); mcr_CVAR_DEFINE_AUTO_CPP(rxyr_triapi_a, "255", 0, ""); mcr_CVAR_DEFINE_AUTO_CPP(rxyr_showreffps, "1", 0, "show only render fps");

M_CVAR_DEFINE_AUTO_CPP( rx_mipscale, "1", 0, "" );

M_CVAR_DEFINE_AUTO_CPP( rx_imgpixfmt, "0", FCVAR_READ_ONLY, "convert to specified pixel format when load an image, recommended to restart to game after change. 0: RGBA8, 1: RGBA4" );

void Cvars_SetCvar( convar_t* cvar, const char* string ) {
  float newValue = atof( string );

  if ( cvar == &rx_drawimageidx )
    cvarMirrors.rx_drawimageidx.value = M_bound( 1, newValue, MAX_IMAGE_COUNT );

  else if ( cvar == &rx_camnearz )
    cvarMirrors.rx_camnearz.value = newValue;

  else if ( cvar == &rx_camfarz )
    cvarMirrors.rx_camfarz.value = newValue;

  else if ( cvar == &rx_mipscale )
    cvarMirrors.rx_mipscale.value = newValue;

  Backend_Cvars_SetCvar( cvar, string, newValue );

  engineFuncs->Cvar_Set( cvar->name, string );
}

void Cvars_SyncCvarMirrors( ) {
  SetCvarMirror( cvarMirrors.rx_drawimageidx, rx_drawimageidx.string );

  SetCvarMirror( cvarMirrors.rx_camnearz, rx_camnearz.string );
  SetCvarMirror( cvarMirrors.rx_camfarz, rx_camfarz.string );

  // SetCvarMirror(cvarMirrors.rxyr_triapi_vert1_x, rxyr_triapi_vert1_x.string); SetCvarMirror(cvarMirrors.rxyr_triapi_vert1_y, rxyr_triapi_vert1_y.string); SetCvarMirror(cvarMirrors.rxyr_triapi_vert1_z, rxyr_triapi_vert1_z.string); SetCvarMirror(cvarMirrors.rxyr_triapi_vert2_x, rxyr_triapi_vert2_x.string); SetCvarMirror(cvarMirrors.rxyr_triapi_vert2_y, rxyr_triapi_vert2_y.string); SetCvarMirror(cvarMirrors.rxyr_triapi_vert2_z, rxyr_triapi_vert2_z.string); SetCvarMirror(cvarMirrors.rxyr_triapi_vert3_x, rxyr_triapi_vert3_x.string); SetCvarMirror(cvarMirrors.rxyr_triapi_vert3_y, rxyr_triapi_vert3_y.string); SetCvarMirror(cvarMirrors.rxyr_triapi_vert3_z, rxyr_triapi_vert3_z.string); SetCvarMirror(cvarMirrors.rxyr_triapi_r, rxyr_triapi_r.string); SetCvarMirror(cvarMirrors.rxyr_triapi_g, rxyr_triapi_g.string); SetCvarMirror(cvarMirrors.rxyr_triapi_b, rxyr_triapi_b.string); SetCvarMirror(cvarMirrors.rxyr_triapi_a, rxyr_triapi_a.string);

  SetCvarMirror( cvarMirrors.rx_mipscale, rx_mipscale.string );

  cvarMirrors.r_pvs_radius = engineFuncs->pfnGetCvarFloat( "r_pvs_radius" );
  cvarMirrors.r_speeds = static_cast<bool>( engineFuncs->pfnGetCvarFloat( "r_speeds" ) );
  cvarMirrors.r_drawentities = static_cast<bool>( engineFuncs->pfnGetCvarFloat( "r_drawentities" ) );

  Backend_Cvars_SyncCvarMirrors( );
}

void Cvars_CvarsInit( ) {
  engineFuncs->Cvar_RegisterVariable( &rx_drawimageidx );

  engineFuncs->Cvar_RegisterVariable( &rx_camnearz );
  engineFuncs->Cvar_RegisterVariable( &rx_camfarz );

  engineFuncs->Cvar_RegisterVariable( &rx_imgpixfmt );

  snprintf( rx_imgpixfmt.string, sizeof( rx_imgpixfmt.string ), "%i", imageMGR.imagePixFmt );

  Backend_Cvars_CvarsInit( );
}
