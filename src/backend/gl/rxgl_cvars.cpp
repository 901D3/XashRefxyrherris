
#include "cvardef.h"

#include "rx_base.h"
#include "rx_cvars.h"

#include "rxgl_cvars.h"
#include "rxgl_image_manager.h"

#include "rx_macros.h"

rxgl_cvarmirrors glCvarMirrors;

M_CVAR_DEFINE_AUTO_CPP( rxgl_ver, "", FCVAR_READ_ONLY, "" );
M_CVAR_DEFINE_AUTO_CPP( rxgl_glslver, "", FCVAR_READ_ONLY, "" );
M_CVAR_DEFINE_AUTO_CPP( rxgl_renderer, "", FCVAR_READ_ONLY, "" );
M_CVAR_DEFINE_AUTO_CPP( rxgl_vendor, "", FCVAR_READ_ONLY, "" );

M_CVAR_DEFINE_AUTO_CPP( rxgl_texinterp, "0", FCVAR_GLCONFIG, "" );

M_CVAR_DEFINE_AUTO_CPP( rxgl_imgdrawcmdssize, "200", FCVAR_GLCONFIG, "" );

M_CVAR_DEFINE_AUTO_CPP( rxgl_maxsurf, "400", FCVAR_GLCONFIG, "" );
M_CVAR_DEFINE_AUTO_CPP( rxgl_maxvert, "800", FCVAR_GLCONFIG, "" );

BACKEND_EXPORT void GL_Cvars_SetCvar( convar_t *cvar, const char *string, float value ) { }

BACKEND_EXPORT void GL_Cvars_SyncCvarMirrors( ) {
  SetCvarMirror( glCvarMirrors.rxgl_imgdrawcmdssize, rxgl_imgdrawcmdssize.string );

  SetCvarMirror( glCvarMirrors.rxgl_maxsurf, rxgl_maxsurf.string );
  SetCvarMirror( glCvarMirrors.rxgl_maxvert, rxgl_maxvert.string );
}

BACKEND_EXPORT void GL_Cvars_CvarsInit( ) {
  engineFuncs->Cvar_RegisterVariable( &rxgl_ver );
  engineFuncs->Cvar_RegisterVariable( &rxgl_glslver );
  engineFuncs->Cvar_RegisterVariable( &rxgl_renderer );
  engineFuncs->Cvar_RegisterVariable( &rxgl_vendor );

  engineFuncs->Cvar_RegisterVariable( &rxgl_texinterp );

  engineFuncs->Cvar_RegisterVariable( &rxgl_imgdrawcmdssize );

  engineFuncs->Cvar_RegisterVariable( &rxgl_maxsurf );
  engineFuncs->Cvar_RegisterVariable( &rxgl_maxvert );

  strcpy( glCvarMirrors.rxgl_ver, GL_GetVersion( ) );
  strcpy( glCvarMirrors.rxgl_glslver, GL_GetGLSLVersion( ) );
  strcpy( glCvarMirrors.rxgl_renderer, GL_GetRenderer( ) );
  strcpy( glCvarMirrors.rxgl_vendor, GL_GetVendor( ) );
}
