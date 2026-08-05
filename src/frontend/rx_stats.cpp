
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "xash3d_types.h"

#include "engine_stuff.h"

#include "rx_cvars.h"
#include "rx_stats.h"

rx_stats_t refstats;

GAME_EXPORT int RSpeeds_Message_( char *buffer, size_t size ) {
#ifdef R_SPEEDS
  if ( cvarMirrors.r_speeds != 0 ) {
    if ( engineFuncs->drawFuncs->R_SpeedsMessage )
      if ( engineFuncs->drawFuncs->R_SpeedsMessage( buffer, size ) )
        return true;

    strncpy( buffer, refstats.rSpeedsStr, size );

    RSpeeds_Clear( );

    return true;
  }
#endif

  return false;
}

void RSpeeds_Printf( char *stringFmt, ... ) {
  char buffer[ 1024 ];

  va_list fmtArgs;
  va_start( fmtArgs, stringFmt );
  vsnprintf( buffer, sizeof( buffer ), stringFmt, fmtArgs );
  va_end( fmtArgs );

  if ( refstats.rSpeedsCount + strlen( buffer ) >= RSPEEDS_USABLESIZE ) return;

  strcpy( &refstats.rSpeedsStr[ refstats.rSpeedsCount ], buffer );
  refstats.rSpeedsCount += strlen( buffer );
}

void RSpeeds_Clear( ) {
  memset( refstats.rSpeedsStr, 0, sizeof( refstats.rSpeedsStr ) );
  refstats.rSpeedsCount = 0;
}
