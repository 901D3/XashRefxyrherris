
#pragma once

#include "xash3d_types.h"
#include "ref_api.h"

#include "rx_defs.h"
#include "rx_backend.h"

extern rx_refctx_t refctx;

extern const char *GetRendererConfigName_( );
extern void *GetProcessAddr_( const char *name );

extern qboolean RSpeeds_Message_( char *outBuffer, size_t size );

extern qboolean SetDisplayTransform_( ref_screen_rotation_t rotate, int x, int y, float scaleX, float scaleY );
extern void OnGammaChange_( qboolean resetGamma );

extern void ClearScreen_( );

extern void StartFrame_( qboolean clearScene );
extern void EndFrame_( );

extern int GetRefParam_( int parm, int arg );

extern void SetupSkyboxImages_( int *skyBoxImageIdx );
extern void SetSkyCloudImages_( int solidSkyImageIdx, int alphaSkyImageIdx );

extern void _GetFogParamsForImage( int imageIdx, uint8_t *r, uint8_t *g, uint8_t *b, uint8_t *density );

extern qboolean ProcessModelRenderData_( model_t *model, qboolean create, const uint8_t *buffer, size_t bufferSize );

extern void Render3DFrame_( const ref_viewpass_s *engineViewPass );

extern void NewMap_( );

extern void UnloadTexturesForModel( model_t *model );

extern void RSpeeds_Printf( char *stringFmt, ... );
extern void RSpeeds_Clear( );

extern void BaseShutdown_( );

extern qboolean BaseInit_( );
