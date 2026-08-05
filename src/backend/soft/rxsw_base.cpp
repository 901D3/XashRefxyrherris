
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#include <chrono>

#include "xash3d_types.h"
#include "const.h"
#include "xash3d_mathlib.h"
#include "ref_api.h"
#include "render_api.h"
#include "ref_params.h"
#include "com_model.h"
#include "q_client.h"
#include "mod_local.h"

#include "engine_stuff.h"

#include "rx_defs.h"

#include "rxsw_base.h"
#include "rxsw_video.h"
#include "rxsw_3d_surface.h"

#include "rx_logsutils.h"

#include "rx_memmgr.h"

#include "rx_macros.h"

BACKEND_EXPORT int Soft_GetBackendEnumerator( ) { return BACKEND_SOFT; }

BACKEND_EXPORT const char* Soft_GetRendererConfigName( ) { return SOFT_CFGNAME; }

BACKEND_EXPORT void Soft_ClearScreen( ) { }
BACKEND_EXPORT void Soft_StartFrame( ) { }
BACKEND_EXPORT void Soft_EndFrame( ) { Soft_Video_BlitScreen( ); }

BACKEND_EXPORT void Soft_InitAllArenas( ) { }
BACKEND_EXPORT void Soft_Shutdown( ) { }
BACKEND_EXPORT bool Soft_Init( ) { return true; }
