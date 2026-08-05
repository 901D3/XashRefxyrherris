
#pragma once

#include "engine_stuff.h"

#define ENGINE_ALLOC( poolptr, size ) engineFuncs->_Mem_Alloc( poolptr, size, static_cast<qboolean>( 0 ), __FILE__, __LINE__ )
#define ENGINE_ALLOC_POOL( name )     engineFuncs->_Mem_AllocPool( name, 0, __FILE__, __LINE__ )

#define ENGINE_FREE( ptr )          engineFuncs->_Mem_Free( ptr, __FILE__, __LINE__ )
#define ENGINE_FREE_POOL( poolptr ) engineFuncs->_Mem_FreePool( poolptr, __FILE__, __LINE__ )

#define ENGINE_REALLOC( poolptr, memptr, size ) engineFuncs->_Mem_Realloc( poolptr, memptr, size, static_cast<qboolean>( 0 ), __FILE__, __LINE__ )

#ifdef _DEBUG
  #define _CRTDBG_MAP_ALLOC
  #include <crtdbg.h>
  #include <stdlib.h>

  #undef ENGINE_ALLOC
  #undef ENGINE_FREE

  #define ENGINE_ALLOC( poolptr, size ) _malloc_dbg( size, _NORMAL_BLOCK, __FILE__, __LINE__ )
  #define ENGINE_FREE( ptr )            _free_dbg( ptr, _NORMAL_BLOCK )
#endif
