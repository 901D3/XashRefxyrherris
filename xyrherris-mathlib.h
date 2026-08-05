
#ifndef XYR_MATHLIB
#define XYR_MATHLIB

#include <string.h>
#include <math.h>

#include "xash3d_mathlib.h"

#include "xyrherris-types.h"

// math consts

#define EPS 1e-6f

// math functions/macros

#define M_floor( a )     ( floorf( a ) )
#define M_fastfloor( a ) ( ( int )( a ) )
#define M_ceil( a )      ( ceilf( a ) )
#define M_round( a )     ( M_floor( a + 0.5f ) )
#define M_fastround( a ) ( M_fastfloor( a + 0.5f ) )

#define M_bound( min, v, max ) ( Q_min( max, Q_max( min, v ) ) )
#define M_absf( a )            ( ( ( a ) < 0.f ) ? -( a ) : ( a ) )
#define M_absi( a )            ( ( ( a ) < 0 ) ? -( a ) : ( a ) )

static inline int M_abstrick( int a ) {
  int mask = a >> 31;
  return ( a ^ mask ) - mask;
}

#define M_swap( a, b, t ) ( ( t ) = ( a ), ( a ) = ( b ), ( b ) = ( t ) )

#define M_normalize( min, a, max ) ( ( ( a ) - ( min ) ) / ( ( max ) - ( min ) ) )
#define M_normalizeP( min, a, p )  ( ( ( a ) - ( min ) ) / ( p ) )

#define M_modulo( a, b )     ( ( a ) - M_floor( ( a ) / ( b ) ) * ( b ) )
#define M_moduloP( a, b, p ) ( ( a ) - ( p ) * ( b ) )

// angles

#define DEG2RADF( a ) ( ( ( a ) * M_PI2_F ) / 360.f )

// NOT INVERSE
static inline void M_Mat3x4Row_AngleVectors( float pitch, float yaw, float roll, float mat[ 12 ] ) {
  float sr, sp, sy, cr, cp, cy;

  SinCos( DEG2RAD( -yaw ), &sy, &cy );
  SinCos( DEG2RAD( -pitch ), &sp, &cp );
  SinCos( DEG2RAD( -roll ), &sr, &cr );

  mat[ 0 ] = cp * cy;
  mat[ 1 ] = cp * sy;
  mat[ 2 ] = sp;

  mat[ 4 ] = -sy * cr + cy * sp * sr;
  mat[ 5 ] = cy * cr + sy * sp * sr;
  mat[ 6 ] = -cp * sr;

  mat[ 8 ] = sy * sr + cy * sp * cr;
  mat[ 9 ] = -cy * sr + sy * sp * cr;
  mat[ 10 ] = cp * cr;
}

// bit stuff

static inline unsigned int M_BitsToBytes( unsigned int bits ) { return ( bits + 7 ) >> 3; }
static inline int M_NextMultiple( int a, int mul ) { return ( ( a + mul - 1 ) / mul ) * mul; }

#define M_GetValueBitwise( value, shift, mask ) ( ( ( value ) & ( mask ) ) >> ( shift ) )

#define M_bit( n )     ( 1U << ( n ) )
#define M_bit64( n )   ( 1ULL << ( n ) )
#define M_bitmask( n ) ( ( 1U << ( n ) ) - 1 )

static inline int M_firstbit( unsigned int bits ) {
  int i;
  for ( i = 0; !( M_bit( i ) & bits ); i++ );
  return i;
}

static inline int M_countbits( unsigned int bits ) {
  int i;
  for ( i = 0; bits; bits >>= 1 ) i += bits & 1;
  return i;
}

// matrix stuff

#define MAT3X4ROW_IDENTITY( a )                               \
  {                                                           \
    a[ 0 ] = 1.f, a[ 1 ] = 0.f, a[ 2 ] = 0.f, a[ 3 ] = 0.f;   \
    a[ 4 ] = 0.f, a[ 5 ] = 1.f, a[ 6 ] = 0.f, a[ 7 ] = 0.f;   \
    a[ 8 ] = 0.f, a[ 9 ] = 0.f, a[ 10 ] = 1.f, a[ 11 ] = 0.f; \
  }

static inline void M_RotatePointfv( float point[ 3 ], float forward[ 3 ], float right[ 3 ], float up[ 3 ], float out[] ) {
  out[ 0 ] = DotProduct( point, right );
  out[ 1 ] = DotProduct( point, up );
  out[ 2 ] = DotProduct( point, forward );
}

static inline void M_RotatePointfv2(
  float pointX, float pointY, float pointZ,
  float forward[ 3 ], float right[ 3 ], float up[ 3 ],
  float out[] ) {

  out[ 0 ] = pointX * right[ 0 ] + pointY * right[ 1 ] + pointZ * right[ 2 ];
  out[ 1 ] = pointX * up[ 0 ] + pointY * up[ 1 ] + pointZ * up[ 2 ];
  out[ 2 ] = pointX * forward[ 0 ] + pointY * forward[ 1 ] + pointZ * forward[ 2 ];
}

static inline void M_RotatePointMat3x4( float point[ 3 ], float mat[ 12 ], float out[ 3 ] ) { M_RotatePointfv( point, &mat[ 0 ], &mat[ 4 ], &mat[ 8 ], out ); }

#define M_DotProduct4( a, b )                ( ( a )[ 0 ] * ( b )[ 0 ] + ( a )[ 1 ] * ( b )[ 1 ] + ( a )[ 2 ] * ( b )[ 2 ] + ( a )[ 3 ] * ( b )[ 3 ] )
#define M_DotProductVectorMat3x4Offs( a, b ) ( ( a )[ 0 ] * ( b )[ 3 ] + ( a )[ 1 ] * ( b )[ 7 ] + ( a )[ 2 ] * ( b )[ 11 ] )
#define M_VectorAddv( i, v1, v2, v3, o )     ( ( o )[ 0 ] = ( i )[ 0 ] + v1, ( o )[ 1 ] = ( i )[ 1 ] + v2, ( o )[ 2 ] = ( i )[ 2 ] + v3 )
#define M_CrossProduct2D( a, b, c )          ( ( ( b )[ 0 ] - ( a )[ 0 ] ) * ( ( c )[ 1 ] - ( a )[ 1 ] ) - ( ( b )[ 1 ] - ( a )[ 1 ] ) * ( ( c )[ 0 ] - ( a )[ 0 ] ) )

#define M_Vec3( a ) { ( a )[ 0 ], ( a )[ 1 ], ( a )[ 2 ] }

static inline void M_Mat4x4Col_Multiply( float a[ 16 ], float b[ 16 ], float o[ 16 ] ) {
  o[ 0 ] = b[ 0 ] * a[ 0 ] + b[ 1 ] * a[ 4 ] + b[ 2 ] * a[ 8 ] + b[ 3 ] * a[ 12 ];
  o[ 1 ] = b[ 0 ] * a[ 1 ] + b[ 1 ] * a[ 5 ] + b[ 2 ] * a[ 9 ] + b[ 3 ] * a[ 13 ];
  o[ 2 ] = b[ 0 ] * a[ 2 ] + b[ 1 ] * a[ 6 ] + b[ 2 ] * a[ 10 ] + b[ 3 ] * a[ 14 ];
  o[ 3 ] = b[ 0 ] * a[ 3 ] + b[ 1 ] * a[ 7 ] + b[ 2 ] * a[ 11 ] + b[ 3 ] * a[ 15 ];

  o[ 4 ] = b[ 4 ] * a[ 0 ] + b[ 5 ] * a[ 4 ] + b[ 6 ] * a[ 8 ] + b[ 7 ] * a[ 12 ];
  o[ 5 ] = b[ 4 ] * a[ 1 ] + b[ 5 ] * a[ 5 ] + b[ 6 ] * a[ 9 ] + b[ 7 ] * a[ 13 ];
  o[ 6 ] = b[ 4 ] * a[ 2 ] + b[ 5 ] * a[ 6 ] + b[ 6 ] * a[ 10 ] + b[ 7 ] * a[ 14 ];
  o[ 7 ] = b[ 4 ] * a[ 3 ] + b[ 5 ] * a[ 7 ] + b[ 6 ] * a[ 11 ] + b[ 7 ] * a[ 15 ];

  o[ 8 ] = b[ 8 ] * a[ 0 ] + b[ 9 ] * a[ 4 ] + b[ 10 ] * a[ 8 ] + b[ 11 ] * a[ 12 ];
  o[ 9 ] = b[ 8 ] * a[ 1 ] + b[ 9 ] * a[ 5 ] + b[ 10 ] * a[ 9 ] + b[ 11 ] * a[ 13 ];
  o[ 10 ] = b[ 8 ] * a[ 2 ] + b[ 9 ] * a[ 6 ] + b[ 10 ] * a[ 10 ] + b[ 11 ] * a[ 14 ];
  o[ 11 ] = b[ 8 ] * a[ 3 ] + b[ 9 ] * a[ 7 ] + b[ 10 ] * a[ 11 ] + b[ 11 ] * a[ 15 ];

  o[ 12 ] = b[ 12 ] * a[ 0 ] + b[ 13 ] * a[ 4 ] + b[ 14 ] * a[ 8 ] + b[ 15 ] * a[ 12 ];
  o[ 13 ] = b[ 12 ] * a[ 1 ] + b[ 13 ] * a[ 5 ] + b[ 14 ] * a[ 9 ] + b[ 15 ] * a[ 13 ];
  o[ 14 ] = b[ 12 ] * a[ 2 ] + b[ 13 ] * a[ 6 ] + b[ 14 ] * a[ 10 ] + b[ 15 ] * a[ 14 ];
  o[ 15 ] = b[ 12 ] * a[ 3 ] + b[ 13 ] * a[ 7 ] + b[ 14 ] * a[ 11 ] + b[ 15 ] * a[ 15 ];
}

static inline void M_Mat3x4Col_Multiply( float a[ 16 ], float b[ 16 ], float o[ 16 ] ) {
  o[ 0 ] = b[ 0 ] * a[ 0 ] + b[ 1 ] * a[ 4 ] + b[ 2 ] * a[ 8 ];
  o[ 1 ] = b[ 0 ] * a[ 1 ] + b[ 1 ] * a[ 5 ] + b[ 2 ] * a[ 9 ];
  o[ 2 ] = b[ 0 ] * a[ 2 ] + b[ 1 ] * a[ 6 ] + b[ 2 ] * a[ 10 ];
  o[ 3 ] = b[ 0 ] * a[ 3 ] + b[ 1 ] * a[ 7 ] + b[ 2 ] * a[ 11 ] + b[ 3 ];

  o[ 4 ] = b[ 4 ] * a[ 0 ] + b[ 5 ] * a[ 4 ] + b[ 6 ] * a[ 8 ];
  o[ 5 ] = b[ 4 ] * a[ 1 ] + b[ 5 ] * a[ 5 ] + b[ 6 ] * a[ 9 ];
  o[ 6 ] = b[ 4 ] * a[ 2 ] + b[ 5 ] * a[ 6 ] + b[ 6 ] * a[ 10 ];
  o[ 7 ] = b[ 4 ] * a[ 3 ] + b[ 5 ] * a[ 7 ] + b[ 6 ] * a[ 11 ] + b[ 7 ];

  o[ 8 ] = b[ 8 ] * a[ 0 ] + b[ 9 ] * a[ 4 ] + b[ 10 ] * a[ 8 ];
  o[ 9 ] = b[ 8 ] * a[ 1 ] + b[ 9 ] * a[ 5 ] + b[ 10 ] * a[ 9 ];
  o[ 10 ] = b[ 8 ] * a[ 2 ] + b[ 9 ] * a[ 6 ] + b[ 10 ] * a[ 10 ];
  o[ 11 ] = b[ 8 ] * a[ 3 ] + b[ 9 ] * a[ 7 ] + b[ 10 ] * a[ 11 ] + b[ 11 ];
}

static inline void M_Mat3x4Row_MutiplyVector( float a[ 3 ], float b[ 12 ], float o[ 3 ] ) {
  o[ 0 ] = a[ 0 ] * b[ 0 ] + a[ 1 ] * b[ 1 ] + a[ 2 ] * b[ 2 ] + b[ 3 ];
  o[ 1 ] = a[ 0 ] * b[ 4 ] + a[ 1 ] * b[ 5 ] + a[ 2 ] * b[ 6 ] + b[ 7 ];
  o[ 2 ] = a[ 0 ] * b[ 8 ] + a[ 1 ] * b[ 9 ] + a[ 2 ] * b[ 10 ] + b[ 11 ];
}

static inline void M_Mat3x4Row_MutiplyVectorNoTransl( float a[ 3 ], float b[ 12 ], float o[ 3 ] ) {
  o[ 0 ] = a[ 0 ] * b[ 0 ] + a[ 1 ] * b[ 1 ] + a[ 2 ] * b[ 2 ];
  o[ 1 ] = a[ 0 ] * b[ 4 ] + a[ 1 ] * b[ 5 ] + a[ 2 ] * b[ 6 ];
  o[ 2 ] = a[ 0 ] * b[ 8 ] + a[ 1 ] * b[ 9 ] + a[ 2 ] * b[ 10 ];
}

static inline void M_Mat3x4Row_TransformPlane( float plane[ 4 ], float mat[ 12 ], float outPlane[ 4 ] ) {
  M_Mat3x4Row_MutiplyVectorNoTransl( plane, mat, outPlane );

  outPlane[ 3 ] = ( outPlane[ 0 ] * mat[ 3 ] + outPlane[ 1 ] * mat[ 7 ] + outPlane[ 2 ] * mat[ 11 ] ) + plane[ 3 ];
}

static inline void M_Mat3x4Row_TransformAABB( float mins[ 3 ], float maxs[ 3 ], float mat[ 12 ], float outMins[ 3 ], float outMaxs[ 3 ] ) {
  outMins[ 0 ] = outMaxs[ 0 ] = mat[ 3 ];
  outMins[ 1 ] = outMaxs[ 1 ] = mat[ 7 ];
  outMins[ 2 ] = outMaxs[ 2 ] = mat[ 11 ];

  for ( int i = 0; i < 3; i++ ) {
    for ( int j = 0; j < 3; j++ ) {
      float e = mat[ i * 4 + j ] * mins[ j ];
      float f = mat[ i * 4 + j ] * maxs[ j ];

      if ( e < f ) {
        outMins[ i ] += e;
        outMaxs[ i ] += f;
      }
      else {
        outMins[ i ] += f;
        outMaxs[ i ] += e;
      }
    }
  }
}

// color stuff

static inline unsigned int M_BlendMultiply( unsigned int a, unsigned int b ) {
  unsigned int t = a * b + 128;
  return ( t + ( t >> 8 ) ) >> 8;
}

static inline unsigned int M_getr( unsigned int value ) { return value & 0xFF; }
static inline unsigned int M_getg( unsigned int value ) { return ( value >> 8 ) & 0xFF; }
static inline unsigned int M_getb( unsigned int value ) { return ( value >> 16 ) & 0xFF; }
static inline unsigned int M_geta( unsigned int value ) { return value >> 24; }

#define M_getr565( value ) ( ( value ) >> 11 )
#define M_getg565( value ) ( ( ( value ) >> 5 ) & 0b111111 )
#define M_getb565( value ) ( ( value ) & 0b11111 )

static inline unsigned int M_packr( unsigned int value ) { return value & 0xFF; }
static inline unsigned int M_packg( unsigned int value ) { return ( value & 0xFF ) << 8; }
static inline unsigned int M_packb( unsigned int value ) { return ( value & 0xFF ) << 16; }
static inline unsigned int M_packa( unsigned int value ) { return value << 24; }

static inline unsigned int M_rgbavg( unsigned int r, unsigned int g, unsigned int b ) { return ( ( r + g + b ) * 85 ) >> 8; }

#define M_IsPowerOf2( x )               !!( ( x ) && !( ( x ) & ( ( x ) - 1 ) ) )
#define M_IsPOTTexture( width, height ) ( M_IsPowerOf2( width ) && M_IsPowerOf2( height ) )

#define Q16_SHIFT 16
#define Q16_1     ( 1U << Q16_SHIFT )
#define Q16_1F    ( ( float )( Q16_1 ) )
#define Q16_HALF  ( 1U << ( Q16_SHIFT >> 1 ) )

#define Q12_SHIFT 12
#define Q12_1     ( 1U << Q12_SHIFT )
#define Q12_1F    ( ( float )( Q12_1 ) )
#define Q12_HALF  ( 1U << ( Q12_SHIFT >> 1 ) )

#define FP_ROUND( a, q ) ( ( ( a ) + ( q ) ) & ( ~( q ) ) )

#define M_8BitMul( a, b ) ( ( ( a ) * ( b ) ) >> 8 )

static inline float M_CalcDeterminant(
  float x0, float y0,
  float x1, float y1,
  float x2, float y2,
  float* dx1, float* dy1,
  float* dx2, float* dy2 ) {

  *dx1 = x1 - x0;
  *dy1 = y1 - y0;

  *dx2 = x2 - x0;
  *dy2 = y2 - y0;

  return ( *dx1 ) * ( *dy2 ) - ( *dx2 ) * ( *dy1 );
}

static inline float M_CalcGradientSteps(
  float x0, float y0,
  float x1, float y1,
  float x2, float y2,
  float grad0, float grad1, float grad2,
  float* gradStepX,
  float* gradStepY,
  float* dx1, float* dy1,
  float* dx2, float* dy2 ) {

  float determin = M_CalcDeterminant( x0, y0, x1, y1, x2, y2, dx1, dy1, dx2, dy2 );
  if ( M_absf( determin ) <= EPS ) return determin;

  float dgrad0 = grad1 - grad0,
        dgrad1 = grad2 - grad0;

  *gradStepX = ( dgrad0 * ( *dy2 ) - dgrad1 * ( *dy1 ) ) / determin;
  *gradStepY = -( dgrad0 * ( *dx2 ) - dgrad1 * ( *dx1 ) ) / determin;

  return determin;
}

static inline void M_CalcGradientStepsPrecomp(
  float dx1, float dy1,
  float dx2, float dy2,
  float determin,
  float grad0, float grad1, float grad2,
  float* gradStepX,
  float* gradStepY ) {

  float dgrad0 = grad1 - grad0,
        dgrad1 = grad2 - grad0;

  *gradStepX = ( dgrad0 * dy2 - dgrad1 * dy1 ) / determin;
  *gradStepY = -( dgrad0 * dx2 - dgrad1 * dx1 ) / determin;
}

#endif
