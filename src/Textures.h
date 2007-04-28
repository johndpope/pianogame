// Synthesia
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

   TrackPanel,

   StatsText,

   PlayStatus,
   PlayStatus2,
   PlayKeys,

   PlayNotesBlackColor,
   PlayNotesBlackShadow,
   PlayNotesWhiteColor,
   PlayNotesWhiteShadow,

   PlayKeyRail,
   PlayKeyShadow,
   PlayKeysBlack,

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

   L"TrackBox",

   L"StatsText",

   L"PlayStatus",
   L"PlayStatus2",
   L"PlayKeys",

   L"PlayNotesBlackColor",
   L"PlayNotesBlackShadow",
   L"PlayNotesWhiteColor",
   L"PlayNotesWhiteShadow",

   L"PlayKeyRail",
   L"PlayKeyShadow",
   L"PlayKeysBlack"

};

#endif
