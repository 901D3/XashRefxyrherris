
#pragma once

#define RSPEEDS_SIZE       2048
#define RSPEEDS_USABLESIZE ( RSPEEDS_SIZE - 1 )

#define R_SPEEDS_LOG( stringFmt, ... ) RSpeeds_Printf( stringFmt "\n", ##__VA_ARGS__ )

typedef struct {
  char rSpeedsStr[ RSPEEDS_SIZE ];
  int rSpeedsCount;

} rx_stats_t;

extern rx_stats_t refstats;

extern int RSpeeds_Message_( char *buffer, size_t size );
extern void RSpeeds_Printf( char *stringFmt, ... );
extern void RSpeeds_Clear( );
