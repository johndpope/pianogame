#include "string_util.h"

#ifdef WIN32
#include "registry.h"
#else
#include <Carbon/Carbon.h>
#endif

using namespace std;

namespace UserSetting
{

#ifdef WIN32

   static bool g_initialized(false);
   static std::wstring g_app_name(L"");

   void Initialize(const std::wstring &app_name)
   {
      if (g_initialized) return;
      g_app_name = app_name;
      g_initialized = true;
   }

   std::wstring Get(const std::wstring &setting, const std::wstring &default_value)
   {
      if (!g_initialized) return default_value;

      Registry reg(Registry::CurrentUser, g_app_name);

      wstring result;
      reg.Read(setting, &result, default_value);

      return result;
   }

   void Set(const std::wstring &setting, const std::wstring &value)
   {
      if (!g_initialized) return;

      Registry reg(Registry::CurrentUser, g_app_name);
      reg.Write(setting, value);
   }

#else

   // MACTODOTEST: UserSettings implementation

   void Initialize(const std::wstring &app_name)
   {
      // Do nothing.  Mac side doesn't need this.
   }

   std::wstring Get(const std::wstring &setting, const std::wstring &default_value)
   {
      CFStringRef val = (CFStringRef)CFPreferencesCopyAppValue(MacStringFromWide(setting).get(), kCFPreferencesCurrentApplication );
      
      if (!val) return default_value;

      // NOTE: This is quite a bit of hassle seeing as how there is
      // really only one line in here that is doing any work.  I would
      // imagine some helper functions would make this shorter.
      const static int BufferSize = 512;
      char buffer[BufferSize];
      for (int i = 0; i < BufferSize; ++i) buffer[i] = 0;
      
      Boolean ret = CFStringGetCString(val, buffer, BufferSize, 0);
      if (!ret) return default_value;
      
      CFRelease(val);
      std::string narrow_result(buffer);
      
      std::wstring return_value(narrow_result.begin(), narrow_result.end());         
      return return_value;
   }
      
   void Set(const std::wstring &setting, const std::wstring &value)
   {
      CFPreferencesSetAppValue(MacStringFromWide(setting).get(), MacStringFromWide(value).get(), kCFPreferencesCurrentApplication);
      CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
   }
      

#endif

}; // End namespace
