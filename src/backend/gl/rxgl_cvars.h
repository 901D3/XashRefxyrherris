
#pragma once

#include "cvardef.h"

#include "engine_stuff.h"

#include "rx_cvars.h"

#include "rx_base.h"

typedef struct {
  char rxgl_ver[ 128 ];
  char rxgl_glslver[ 128 ];
  char rxgl_renderer[ 128 ];
  char rxgl_vendor[ 128 ];

  CvarMirror<int, 8> rxgl_imgdrawcmdssize;

  CvarMirror<int, 8> rxgl_maxsurf;
  CvarMirror<int, 8> rxgl_maxvert;

} rxgl_cvarmirrors;

extern rxgl_cvarmirrors glCvarMirrors;

extern convar_t rxgl_ver;
extern convar_t rxgl_glslver;
extern convar_t rxgl_renderer;
extern convar_t rxgl_vendor;

extern void GL_Cvars_SetCvar( convar_t *cvar, const char *string, float value );
extern void GL_Cvars_SyncCvarMirrors( );
extern void GL_Cvars_CvarsInit( );
