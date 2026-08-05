
#include <stdlib.h>
#include <stdint.h>

#include "rxsw_triapi.h"

#include "rx_macros.h"

BACKEND_EXPORT void Soft_TriAPI_SetBlendMode( int blendMode ) { }
BACKEND_EXPORT void Soft_TriAPI_Immediate_Begin( int drawMode ) { }
BACKEND_EXPORT void Soft_TriAPI_Immediate_End( ) { }
BACKEND_EXPORT void Soft_TriAPI_CullFace( int cullMode ) { }
BACKEND_EXPORT bool Soft_TriAPI_Init( ) { return true; };
