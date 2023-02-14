#include "debug.h"

#ifdef _MSC_VER
#include <crtdbg.h>
#include <windows.h>
#endif

#if defined(MEMORY_LEAK_TRACKING) && defined(_MSC_VER)
#ifndef NDEBUG
#define MEMORY_LEAK_TRACKING_MSVC
#endif
#endif  


void init_debug()
  {
#if defined(MEMORY_LEAK_TRACKING_MSVC)
// For more thorough checking:
// _CRTDBG_DELAY_FREE_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF
auto flags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
_CrtSetDbgFlag(flags | _CRTDBG_ALLOC_MEM_DF);
#endif
  }


void close_debug()
  {
#if defined(MEMORY_LEAK_TRACKING_MSVC)
  auto flags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
  flags &= ~_CRTDBG_DELAY_FREE_MEM_DF;
  flags |= _CRTDBG_LEAK_CHECK_DF;
  _CrtSetDbgFlag(flags);
#endif
  }