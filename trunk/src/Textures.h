
// Copyright (c)2007 Nicholas Piegdon
// See license.txt for license information

#ifndef __TEXTURES_H
#define __TEXTURES_H

enum Texture
{
   TitleLogo,
   InterfaceButtons,

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
   L"title_Logo",
   L"InterfaceButtons",

   L"score_RetrySong",
   L"title_ChooseTracks",
   L"title_Exit",
   L"tracks_BackToTitle",
   L"tracks_PlaySong",

   L"title_InputBox",
   L"title_OutputBox",
   L"title_SongBox",

   L"trackbox",

   L"stats_text",

   L"play_Status",
   L"play_Status2",
   L"play_Keys",

   L"play_NotesBlackColor",
   L"play_NotesBlackShadow",
   L"play_NotesWhiteColor",
   L"play_NotesWhiteShadow",

   L"play_KeyRail",
   L"play_KeyShadow",
   L"play_KeysBlack"
};

#endif
