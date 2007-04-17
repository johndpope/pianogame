// Piano Hero
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#ifndef __TRACK_PROPERTIES_H
#define __TRACK_PROPERTIES_H

#include "Renderer.h"

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

const static Color TrackColorNoteWhite[TrackColorCount] = {
   { 114, 159, 207, 0xFF },
   { 138, 226,  52, 0xFF },
   { 252, 175,  62, 0xFF },
   { 252, 235,  87, 0xFF },
   { 173, 127, 168, 0xFF },
   { 238,  94,  94, 0xFF },

   { 180, 180, 180, 0xFF },
   {  60,  60,  60, 0xFF }
};

const static Color TrackColorNoteHit[TrackColorCount] = {
   { 192, 222, 255, 0xFF },
   { 203, 255, 152, 0xFF },
   { 255, 216, 152, 0xFF },
   { 255, 247, 178, 0xFF },
   { 255, 218, 251, 0xFF },
   { 255, 178, 178, 0xFF },

   { 180, 180, 180, 0xFF },
   {  60,  60,  60, 0xFF }
};

const static Color TrackColorNoteBlack[TrackColorCount] = {
   {  52, 101, 164, 0xFF },
   {  96, 176,  19, 0xFF },
   { 245, 121,   0, 0xFF },
   { 218, 195,   0, 0xFF },
   { 117,  80, 123, 0xFF },
   { 233,  49,  49, 0xFF },

   { 120, 120, 120, 0xFF },
   {  40,  40,  40, 0xFF }
};

const static Color TrackColorNoteBorder[TrackColorCount] = {
   {  32,  64, 115, 0xFF },
   {  58, 114,   6, 0xFF },
   { 166,  72,   0, 0xFF },
   { 145, 120,   0, 0xFF },
   {  92,  53, 102, 0xFF },
   { 164,   0,   0, 0xFF },

   {  40,  40,  40, 0xFF },
   {   0,   0,   0, 0xFF }
};

struct TrackProperties
{
   TrackProperties() : mode(ModeNotPlayed), color(TangoSkyBlue) { }

   TrackMode mode;
   TrackColor color;
};

#endif