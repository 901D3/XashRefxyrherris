
#pragma once

#include <stdint.h>
#include <stdlib.h>

#include "cvardef.h"

#include "const.h"
#include "crtlib.h"

#define M_CVAR_DEFINE_AUTO_CPP( convarVar, string, flags, description )                                                       \
  convar_t convarVar = { #convarVar, string, flags, 0.f, reinterpret_cast<convar_t*>( CVAR_SENTINEL ), description, nullptr }

template <typename T, int StringLength>
struct CvarMirror {
  char string[ StringLength ];
  T value{ };
};

extern convar_t rx_video_bpp;
extern convar_t rx_drawimageidx;
extern convar_t rx_camnearz;
extern convar_t rx_camfarz;
extern convar_t rx_mipscale;
extern convar_t rx_imgpixfmt;

typedef struct {
  CvarMirror<int, 8> rx_video_bpp;

  CvarMirror<int, 8> rxyr_light_bit;

  CvarMirror<int, 8> rx_drawimageidx;

  CvarMirror<int, 8> rx_imgpixfmt;

  CvarMirror<float, 16> rx_camnearz;
  CvarMirror<float, 16> rx_camfarz;

  CvarMirror<float, 8> rx_mipscale;

  CvarMirror<int, 2> rx_windingorder;

  float r_pvs_radius;
  bool r_speeds;
  bool r_drawentities;

} rx_cvarmirrors_t;

extern rx_cvarmirrors_t cvarMirrors;

template <typename T, int stringLength>
static inline void SetCvarMirror( CvarMirror<T, stringLength> &cvarMirror, const char* string ) {
  if ( strcmp( cvarMirror.string, string ) != 0 ) {
    strncpy( cvarMirror.string, string, stringLength - 1 );
    cvarMirror.string[ stringLength - 1 ] = '\0';

    cvarMirror.value = Q_atof( string );
  }
}

template <typename T, int stringLength>
static inline void SetCvarMirrorWCallBack( CvarMirror<T, stringLength> &cvarMirror, const char* string, void( *callback ) ) {
  if ( strcmp( cvarMirror.string, string ) != 0 ) {
    strncpy( cvarMirror.string, string, stringLength - 1 );
    cvarMirror.string[ stringLength - 1 ] = '\0';

    cvarMirror.value = Q_atoi( string );
    callback( );
  }
}

extern void Cvars_SetCvar( convar_t* cvar, const char* string );
extern void Cvars_SyncCvarMirrors( );
extern void Cvars_CvarsInit( );
