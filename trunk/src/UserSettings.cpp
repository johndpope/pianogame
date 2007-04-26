#include "registry.h"
#include "string_util.h"

using namespace std;

namespace UserSetting
{
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

}; // End namespace