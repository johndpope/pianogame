// Synthesia
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#include "registry.h"
#include <cassert>

using std::wstring;

Registry::Registry(const RootKey rootKey, const wstring program, const wstring company)
{
   good = true;
   key = NULL;

   const wstring run_buf = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
   wstring buffer = L"Software\\";

   if (rootKey != CU_Run && rootKey != LM_Run)
   {
      if (program.length() > 0)
      {
         // Handle passing in only one string to write to the company root
         if (company.length() > 0) buffer += company + L"\\" + program;
         else buffer += program;
      }
      else good = false;
   }

   if (good)
   {
      long result = 0;
      DWORD disposition;

      // Open the requested key
      switch(rootKey)
      {
      case CurrentUser:
         result = RegCreateKeyEx(HKEY_CURRENT_USER, buffer.c_str(), 0, L"REG_DWORD", REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &key, &disposition);
         if (result != ERROR_SUCCESS) good = false;
         break;

      case LocalMachine:
         result = RegCreateKeyEx(HKEY_LOCAL_MACHINE, buffer.c_str(), 0, L"REG_DWORD", REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &key, &disposition);
         if (result != ERROR_SUCCESS) good = false;
         break;

      case CU_Run:
         result = RegCreateKeyEx(HKEY_CURRENT_USER, run_buf.c_str(), 0, L"REG_DWORD", REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &key, &disposition);
         if (result != ERROR_SUCCESS) good = false;
         break;

      case LM_Run:
         result = RegCreateKeyEx(HKEY_LOCAL_MACHINE, run_buf.c_str(), 0, L"REG_DWORD", REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &key, &disposition);
         if (result != ERROR_SUCCESS) good = false;
         break;
      }
   }

   assert(good);
}

Registry::~Registry()
{
   RegCloseKey(key);
}

void Registry::Delete(const wstring keyName)
{
   if (!good) return;
   RegDeleteValue(key, keyName.c_str());
}


void Registry::Write(const wstring keyName, const wstring value)
{
   if (!good) return;
   RegSetValueEx(key, keyName.c_str(), 0, REG_SZ, (LPBYTE)value.c_str(), (DWORD)( (value.length()+1)*2 ));
}

void Registry::Write(const wstring keyName, const bool value)
{
   if (!good) return;

   int val = (value)?1:0;
   RegSetValueEx(key, keyName.c_str(), 0, REG_DWORD, (LPBYTE)&val, sizeof(DWORD));
}

void Registry::Write(const wstring keyName, const long value)
{
   if (!good) return;
   RegSetValueEx(key, keyName.c_str(), 0, REG_DWORD, (LPBYTE)&value, sizeof(DWORD));
}

void Registry::Write(const wstring keyName, const int value)
{
   if (!good) return;
   RegSetValueEx(key, keyName.c_str(), 0, REG_DWORD, (LPBYTE)&value, sizeof(DWORD));
}

const bool Registry::Read(const wstring keyName, wstring *out, const wstring defaultValue) const
{
   // Default the return value immediately
   *out = defaultValue;
   if (!good) return false;

   // Read the value once to get the size of the string
   long result = 0;
   DWORD size = 0;
   result = RegQueryValueEx(key, keyName.c_str(), 0, NULL, NULL, &size);

   // Read the value again to get the actual string
   if (result == ERROR_SUCCESS)
   {
      wchar_t *data = new wchar_t[size + 1];
      if (!data) return false;

      result = RegQueryValueEx(key, keyName.c_str(), 0, NULL, (LPBYTE)data, &size);

      if (result == ERROR_SUCCESS) *out = wstring(data);

      if (data) delete[] data;
      data = 0;
   }

   // 'out' would have only been set on success, otherwise the
   // default still exists in 'out', so we're all set
   return (result == ERROR_SUCCESS);
}

const bool Registry::Read(const wstring keyName, bool *out, const bool defaultValue) const
{
   // Default the return value immediately
   *out = defaultValue;
   if (!good) return false;

   DWORD data = 0;
   DWORD dataSize = sizeof(DWORD);

   const long result = RegQueryValueEx(key, keyName.c_str(), 0, NULL, (LPBYTE)&data, &dataSize);
   if (result == ERROR_SUCCESS) *out = !(data == 0);

   return (result == ERROR_SUCCESS);
}

const bool Registry::Read(const wstring keyName, long *out, const long defaultValue) const
{
   // Default the return value immediately
   *out = defaultValue;
   if (!good) return false;

   DWORD data = 0;
   DWORD dataSize = sizeof(DWORD);

   const long result = RegQueryValueEx(key, keyName.c_str(), 0, NULL, (LPBYTE)&data, &dataSize);
   if (result == ERROR_SUCCESS) *out = data;

   return (result == ERROR_SUCCESS);
}

const bool Registry::Read(const wstring keyName, int *out, const int defaultValue) const
{
   // Default the return value immediately
   *out = defaultValue;
   if (!good) return false;

   DWORD data = 0;
   DWORD dataSize = sizeof(DWORD);

   const long result = RegQueryValueEx(key, keyName.c_str(), 0, NULL, (LPBYTE)&data, &dataSize);
   if (result == ERROR_SUCCESS) *out = (signed)data;

   return (result == ERROR_SUCCESS);
}

