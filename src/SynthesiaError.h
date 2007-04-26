// Synthesia
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#ifndef __SYNTHESIA_ERROR_H
#define __SYNTHESIA_ERROR_H

#include <iostream>
#include <string>

enum SynthesiaErrorCode
{
   Error_BadPianoType,
   Error_BadGameState
};

class SynthesiaError : public std::exception
{
public:
   SynthesiaError(SynthesiaErrorCode error) : m_error(error) { }
   std::wstring GetErrorDescription() const;

   const SynthesiaErrorCode m_error;

private:
   SynthesiaError operator=(const SynthesiaError&);
};

#endif
