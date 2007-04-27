// Synthesia
// Copyright (c)2007 Nicholas Piegdon
// See license.txt for license information

#ifndef __USER_SETTINGS_H
#define __USER_SETTINGS_H

#include <string>

namespace UserSetting
{
   // This must be called exactly once before any of the following will work
   void Initialize(const std::wstring &app_name);

   std::wstring Get(const std::wstring &setting, const std::wstring &default_value);
   void Set(const std::wstring &setting, const std::wstring &value);
};

#endif
