// Synthesia
// Copyright (c)2007 Nicholas Piegdon
// See license.txt for license information

#include "SynthesiaError.h"
#include "string_util.h"

using namespace std;

std::wstring SynthesiaError::GetErrorDescription() const
{
   switch (m_error)
   {
   case Error_BadPianoType:                return L"Bad piano type specified.";
   case Error_BadGameState:                return L"Internal Error: Synthesia entered bad game state!";

   default:                                return WSTRING(L"Unknown SynthesiaError Code (" << m_error << L").");
   }
}

