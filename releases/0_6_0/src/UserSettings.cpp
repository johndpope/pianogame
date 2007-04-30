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

   void Initialize(const std::wstring &app_name)
   {
      // Do nothing.  Mac side doesn't need this.
   }

   std::wstring Get(const std::wstring &setting, const std::wstring &default_value)
   {
      CFStringRef val = (CFStringRef)CFPreferencesCopyAppValue(MacStringFromWide(setting).get(), kCFPreferencesCurrentApplication );
      if (!val) return default_value;
      
      std::wstring ret = WideFromMacString(val);
      CFRelease(val);

      return ret;
   }
      
   void Set(const std::wstring &setting, const std::wstring &value)
   {
      CFPreferencesSetAppValue(MacStringFromWide(setting).get(), MacStringFromWide(value).get(), kCFPreferencesCurrentApplication);
      CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
   }
      

#endif

}; // End namespace
