#ifdef WIN32
#include <Windows.h>
#else
// TODO!
#endif

namespace Time
{
   unsigned long GetMilliseconds()
   {
      unsigned long milliseconds = 0;
      
      #ifdef WIN32
         timeGetTime();
      #else
         // TODO!
      #endif
      
      return milliseconds;
   }

}; // End namespace