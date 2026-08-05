
#ifndef XYR_TYPES
#define XYR_TYPES

typedef char int8;
typedef unsigned char uint8;
typedef short int16;
typedef unsigned short uint16;
typedef int int32;
typedef unsigned int uint32;
typedef long long int64;
typedef unsigned long long uint64;

#define SIZEOF_CHAR  sizeof( char )
#define SIZEOF_SHORT sizeof( short )
#define SIZEOF_INT   sizeof( int )
#define SIZEOF_INT64 sizeof( long long )

#define SIZEOF_FLOAT  sizeof( float )
#define SIZEOF_DOUBLE sizeof( double )

#define SIZEOF_INTPTR32 4

typedef uint32 pixrgba8_t;
typedef uint16 pixrgba4_t;
typedef uint8 pixbyte_t;

#endif
