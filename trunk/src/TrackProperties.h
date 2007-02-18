// Piano Hero
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#ifndef __TRACK_PROPERTIES_H
#define __TRACK_PROPERTIES_H

#include <windows.h>

enum TrackMode
{
   ModePlayedAutomatically,
   ModeYouPlay,
   ModePlayedButHidden,
   ModeNotPlayed,

   TrackModeCount
};

const static wchar_t* TrackModeText[TrackModeCount] =
{
   L"Played Automatically",
   L"You Play",
   L"Played But Hidden",
   L"Not Played"
};

// Based on the Open Source icon theme "Tango" color scheme
// with a few changes.  (e.g. Chameleon NoteBlack is a little
// darker to distinguish it from NoteWhite, ScarletRed is a
// little brighter to make it easier on the eyes, etc.)
const static int TrackColorCount = 8;
const static int UserSelectableColorCount = TrackColorCount - 2;
enum TrackColor
{
   TangoSkyBlue = 0,
   TangoChameleon,
   TangoOrange,
   TangoButter,
   TangoPlum,
   TangoScarletRed,

   FlatGray,
   MissedNote
};

const static COLORREF TrackColorNoteWhite[TrackColorCount] = {
   RGB(114, 159, 207),
   RGB(138, 226,  52),
   RGB(252, 175,  62),
   RGB(252, 235,  87),
   RGB(173, 127, 168),
   RGB(238,  94,  94),

   RGB(180, 180, 180),
   RGB( 60,  60,  60)
};

const static COLORREF TrackColorNoteHit[TrackColorCount] = {
   RGB(192, 222, 255),
   RGB(203, 255, 152),
   RGB(255, 216, 152),
   RGB(255, 247, 178),
   RGB(255, 218, 251),
   RGB(255, 178, 178),

   RGB(180, 180, 180),
   RGB( 60,  60,  60)
};

const static COLORREF TrackColorNoteBlack[TrackColorCount] = {
   RGB( 52, 101, 164),
   RGB( 96, 176,  19),
   RGB(245, 121,   0),
   RGB(218, 195,   0),
   RGB(117,  80, 123),
   RGB(233,  49,  49),

   RGB(120, 120, 120),
   RGB( 40,  40,  40)
};

const static COLORREF TrackColorNoteBorder[TrackColorCount] = {
   RGB( 32,  64, 115),
   RGB( 58, 114,   6),
   RGB(166,  72,   0),
   RGB(145, 120,   0),
   RGB( 92,  53, 102),
   RGB(164,   0,   0),

   RGB( 40,  40,  40),
   RGB(  0,   0,   0)
};

struct TrackProperties
{
   TrackProperties() : mode(ModeNotPlayed), color(TangoSkyBlue) { }

   TrackMode mode;
   TrackColor color;
};

#endif