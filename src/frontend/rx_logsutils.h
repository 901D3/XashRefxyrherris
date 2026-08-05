
#pragma once

#include <string.h>
#include "engine_stuff.h"
#include "com_strings.h"

#define __FILENAME__ ( strrchr( __FILE__, '/' ) ? strrchr( __FILE__, '/' ) + 1 : ( strrchr( __FILE__, '\\' ) ? strrchr( __FILE__, '\\' ) + 1 : __FILE__ ) )

#define dlog( fmt, ... ) (                       \
  engineFuncs->Con_Printf(                       \
    S_MAGENTA "REF:[%s:%i]>" S_DEFAULT fmt "\n", \
    __FILENAME__,                                \
    __LINE__,                                    \
    ##__VA_ARGS__ ) )

#define errlog( fmt, ... ) (     \
  engineFuncs->Con_Printf(       \
    "REF_ERR:[%s:%i]>" fmt "\n", \
    __FILENAME__,                \
    __LINE__,                    \
    ##__VA_ARGS__ ) )

#define warnlog( fmt, ... ) (     \
  engineFuncs->Con_Printf(        \
    "REF_WARN:[%s:%i]>" fmt "\n", \
    __FILENAME__,                 \
    __LINE__,                     \
    ##__VA_ARGS__ ) )

#define gllog( fmt, ... ) ( \
  engineFuncs->Con_Printf(  \
    "GL:[%s:%i]>" fmt "\n", \
    __FILENAME__,           \
    __LINE__,               \
    ##__VA_ARGS__ ) )

#define glerrlog( fmt, ... ) (  \
  engineFuncs->Con_Printf(      \
    "GL_ERR:[%s:%i]>" fmt "\n", \
    __FILENAME__,               \
    __LINE__,                   \
    ##__VA_ARGS__ ) )

#define glwarnlog( fmt, ... ) (  \
  engineFuncs->Con_Printf(       \
    "GL_WARN:[%s:%i]>" fmt "\n", \
    __FILENAME__,                \
    __LINE__,                    \
    ##__VA_ARGS__ ) )

#define M_HostError( fmt, ... )     \
  {                                 \
    dlog( fmt, ##__VA_ARGS__ );     \
    engineFuncs->Host_Error(        \
      "HOST_ERR:[%s:%i]>" fmt "\n", \
      __FILENAME__,                 \
      __LINE__,                     \
      ##__VA_ARGS__ );              \
  }
