#ifdef WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#else
// MACTODO!
#endif

namespace Time
{
   unsigned long GetMilliseconds()
   {
      unsigned long milliseconds = 0;

#ifdef WIN32
      milliseconds = timeGetTime();
#else
      // MACTODO!
#endif

      return milliseconds;
   }

}; // End namespace