// Synthesia
// Copyright (c)2007 Nicholas Piegdon
// See license.txt for license information

#ifndef __REGISTRY_H
#define __REGISTRY_H

#include <string>

#ifdef WIN32
#include <Windows.h>
#endif

// Registry simplifies reading/writing the Windows registry
// (It currently does not support enumerating or deleting)
class Registry
{
public:
   enum RootKey { CurrentUser, LocalMachine, CU_Run, LM_Run };

   // Company is optional.  Read/Write will use [rootKey]/Software/[program] if
   // not supplied, or [rootKey]/Software/[company]/[program] if it is supplied.
   //
   // In the event of CU_Run or LM_Run, both 'program' and 'company' are ignored.
   Registry(const RootKey rootKey, const std::wstring program, const std::wstring company = L"");
   ~Registry();

   // If the key was found and read successfully, function will return true,
   // otherwise 'out' will be filled with defaultValue, and function will
   // return false. (Regardless of return value, 'out' will always be usable)
   const bool Read(const std::wstring keyName, std::wstring *out, const std::wstring defaultValue) const;
   const bool Read(const std::wstring keyName, bool *out, const bool defaultValue) const;
   const bool Read(const std::wstring keyName, long *out, const long defaultValue) const;
   const bool Read(const std::wstring keyName, int  *out, const int  defaultValue) const;

   void Write(const std::wstring keyName, const std::wstring value);
   void Write(const std::wstring keyName, const bool value);
   void Write(const std::wstring keyName, const long value);
   void Write(const std::wstring keyName, const int  value);

   void Delete(const std::wstring keyName);

private:
   bool good;
   HKEY key;
};

#endif
