
// Copyright (c)2007 Nicholas Piegdon
// See license.txt for license information

#ifndef __PIANOGAME_ERROR_H
#define __PIANOGAME_ERROR_H

#include <iostream>
#include <string>

enum PianoGameErrorCode
{
   Error_StringSpecified,
   
   Error_BadPianoType,
   Error_BadGameState
};

class PianoGameError : public std::exception
{
public:

   // TODO: This would be a sweet place to add stack-trace information...

   PianoGameError(PianoGameErrorCode error) : m_error(error), m_optional_string(L"") { }
   PianoGameError(const std::wstring error) : m_error(Error_StringSpecified), m_optional_string(error) { }
   std::wstring GetErrorDescription() const;

   ~PianoGameError() throw() { }

   const PianoGameErrorCode m_error;

private:
   const std::wstring m_optional_string;
   PianoGameError operator=(const PianoGameError&);
};

#endif
