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
   { 114, 159, 207 },
   { 138, 226,  52 },
   { 252, 175,  62 },
   { 252, 235,  87 },
   { 173, 127, 168 },
   { 238,  94,  94 },

   { 180, 180, 180 },
   {  60,  60,  60 }
};

const static Color TrackColorNoteHit[TrackColorCount] = {
   { 192, 222, 255 },
   { 203, 255, 152 },
   { 255, 216, 152 },
   { 255, 247, 178 },
   { 255, 218, 251 },
   { 255, 178, 178 },

   { 180, 180, 180 },
   {  60,  60,  60 }
};

const static Color TrackColorNoteBlack[TrackColorCount] = {
   {  52, 101, 164 },
   {  96, 176,  19 },
   { 245, 121,   0 },
   { 218, 195,   0 },
   { 117,  80, 123 },
   { 233,  49,  49 },

   { 120, 120, 120 },
   {  40,  40,  40 }
};

const static Color TrackColorNoteBorder[TrackColorCount] = {
   {  32,  64, 115 },
   {  58, 114,   6 },
   { 166,  72,   0 },
   { 145, 120,   0 },
   {  92,  53, 102 },
   { 164,   0,   0 },

   {  40,  40,  40 },
   {   0,   0,   0 }
};

struct TrackProperties
{
   TrackProperties() : mode(ModeNotPlayed), color(TangoSkyBlue) { }

   TrackMode mode;
   TrackColor color;
};

#endif