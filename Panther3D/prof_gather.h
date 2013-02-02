#ifndef INC_PROFILER_LOWLEVEL_H
#define INC_PROFILER_LOWLEVEL_H

#ifdef __cplusplus
  #define Prof_C               "C"
  #define Prof_extern_C        extern "C"
  #define Prof_dummy_declare
#else
  #define Prof_C
  #define Prof_extern_C
  #define Prof_dummy_declare   int Prof_dummy_dec =
#endif

#ifdef WIN32
  //#include "prof_win32.h"
#else
  #error "need to define Prof_get_timestamp() and Prof_Int64"
#endif

#include "profiler.h"

extern Prof_C Prof_Zone_Stack * Prof_stack; // current Zone stack
extern Prof_C Prof_Zone_Stack   Prof_dummy; // parent never matches

extern Prof_C Prof_Zone_Stack * Prof_StackAppend(Prof_Zone *zone);
// return the zone stack created by pushing 'zone' on the current

#endif // INC_PROFILER_LOWLEVEL_H
