// Piano Hero
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#ifndef __PIANO_HERO_ERROR_H
#define __PIANO_HERO_ERROR_H

#include <iostream>
#include <string>

enum PianoHeroErrorCode
{
   Error_BadPianoType,
   Error_BadGameState
};

class PianoHeroError : public std::exception
{
public:
   PianoHeroError(PianoHeroErrorCode error) : m_error(error) { }
   std::wstring GetErrorDescription() const;

   const PianoHeroErrorCode m_error;
};

#endif
