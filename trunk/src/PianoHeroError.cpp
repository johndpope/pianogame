// Piano Hero
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#include "PianoHeroError.h"
#include "string_util.h"

using namespace std;

std::wstring PianoHeroError::GetErrorDescription() const
{
   switch (m_error)
   {
   case Error_BadPianoType:                return L"Bad piano type specified. (Piano code isn't 61, 76, or 88 key.)";
   case Error_BadGameState:                return L"Internal Error: Piano Hero entered bad game state!";

   default:                                return WSTRING(L"Unknown PianoHeroError Code (" << m_error << L").");
   }
}

