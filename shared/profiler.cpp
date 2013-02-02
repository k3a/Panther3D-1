#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "profiler.h"

Prof_StackAppendFn Prof_StackAppendPTR;
Prof_Zone_Stack * Prof_dummyPTR=NULL;  // impossible parent
Prof_Zone_Stack ** Prof_stackPTR=NULL;