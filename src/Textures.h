// Piano Hero
// Copyright (c)2007 Nicholas Piegdon
// See license.txt for license information

#ifndef __TEXTURES_H
#define __TEXTURES_H

enum Texture
{
   TitleLogo,
   InterfaceButtons,

   KeyboardHelp,
   GameMusicThemes,

   ButtonRetrySong,
   ButtonChooseTracks,
   ButtonExit,
   ButtonBackToTitle,
   ButtonPlaySong,

   InputBox,
   OutputBox,
   SongBox,

   TrackBox,

   _TextureEnumCount
};

const static wchar_t* TextureResourceNames[_TextureEnumCount] =
{
   L"TitleLogo",
   L"InterfaceButtons",

   L"KeyboardHelp",
   L"GameMusicThemes",

   L"ButtonRetrySong",
   L"ButtonChooseTracks",
   L"ButtonExit",
   L"ButtonBackToTitle",
   L"ButtonPlaySong",

   L"InputBox",
   L"OutputBox",
   L"SongBox",

   L"TrackBox"
};

#endif
