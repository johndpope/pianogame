
// Copyright (c)2007 Nicholas Piegdon
// See license.txt for license information

#ifndef __SYNTHESIA_ERROR_H
#define __SYNTHESIA_ERROR_H

#include <iostream>
#include <string>

enum SynthesiaErrorCode
{
   Error_StringSpecified,
   
   Error_BadPianoType,
   Error_BadGameState
};

class SynthesiaError : public std::exception
{
public:

   // TODO: This would be a sweet place to add stack-trace information...

   SynthesiaError(SynthesiaErrorCode error) : m_error(error), m_optional_string(L"") { }
   SynthesiaError(const std::wstring error) : m_error(Error_StringSpecified), m_optional_string(error) { }
   std::wstring GetErrorDescription() const;

   ~SynthesiaError() throw() { }

   const SynthesiaErrorCode m_error;

private:
   const std::wstring m_optional_string;
   SynthesiaError operator=(const SynthesiaError&);
};

#endif
