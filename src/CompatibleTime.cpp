#ifdef WIN32
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
         timeGetTime();
      #else
         // MACTODO!
      #endif
      
      return milliseconds;
   }

}; // End namespace