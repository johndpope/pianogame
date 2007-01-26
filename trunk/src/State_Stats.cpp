// Piano Hero
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#include "State_Stats.h"
#include "State_TrackSelection.h"
#include "State_Playing.h"

#include <iomanip>

using namespace std;

void StatsState::Init()
{
   m_back_button = ButtonState(Layout::ScreenMarginX,
      GetStateHeight() - Layout::ScreenMarginX - Layout::ButtonHeight,
      Layout::ButtonWidth, Layout::ButtonHeight);

   m_continue_button = ButtonState(GetStateWidth() - Layout::ScreenMarginX - Layout::ButtonWidth,
      GetStateHeight() - Layout::ScreenMarginX - Layout::ButtonHeight,
      Layout::ButtonWidth, Layout::ButtonHeight);
}

void StatsState::Update()
{
   MouseInfo mouse = Mouse();
   
   m_continue_button.Update(mouse);
   m_back_button.Update(mouse);

   if (IsKeyPressed(KeyEscape) || m_back_button.hit)
   {
      ChangeState(new TrackSelectionState(m_state));
      return;
   }

   if (IsKeyPressed(KeyEnter) || m_continue_button.hit)
   {
      ChangeState(new PlayingState(m_state));
      return;
   }
}

void StatsState::Draw(HDC hdc) const
{
   Layout::DrawTitle(hdc, m_state.song_title);
   Layout::DrawHorizontalRule(hdc, GetStateWidth(), Layout::ScreenMarginY);
   Layout::DrawHorizontalRule(hdc, GetStateWidth(), GetStateHeight() - Layout::ScreenMarginY);

   Layout::DrawButton(hdc, m_continue_button, L"Retry Song", 28);
   Layout::DrawButton(hdc, m_back_button, L"Track Selection", 13);

   const static COLORREF Title = RGB(114, 159, 207);
   const static COLORREF Highlight = RGB(138, 226, 52);

   const SongStatistics &s = m_state.stats;

   double hit_percent = 0.0;
   if (s.notes_user_could_have_played > 0)
   {
      hit_percent = 100.0 * (s.notes_user_actually_played / (s.notes_user_could_have_played * 1.0));
   }

   std::wstring grade = L"F";
   if (hit_percent >= 50) grade = L"D-";
   if (hit_percent >= 55) grade = L"D";
   if (hit_percent >= 63) grade = L"D+";
   if (hit_percent >= 70) grade = L"C-";
   if (hit_percent >= 73) grade = L"C";
   if (hit_percent >= 77) grade = L"C+";
   if (hit_percent >= 80) grade = L"B-";
   if (hit_percent >= 83) grade = L"B";
   if (hit_percent >= 87) grade = L"B+";
   if (hit_percent >= 90) grade = L"A-";
   if (hit_percent >= 93) grade = L"A";
   if (hit_percent >= 97) grade = L"A+";
   if (hit_percent >= 99) grade = L"A++";
   if (hit_percent >= 100) grade = L"A+++";

   int left = GetStateWidth() / 2 - (300 / 2);
   const static int InstructionsY = 234;
   
   TextWriter grade_text(left + 110, InstructionsY + 43, hdc, false, 72);
   grade_text << grade;
   
   TextWriter score(left, InstructionsY, hdc, false, Layout::TitleFontSize);
   score << Text(L"Song Statistics", Title) << newline
      << newline 
      << newline 
      << Text(L"Grade: ", Gray) << newline
      << newline
      << newline
      << Text(L"Score: ", Gray) << WSTRING(static_cast<int>(s.score)) << newline
      << Text(L"Note Hit Percent: ", Gray) << WSTRING(static_cast<int>(hit_percent) << L" %") << newline
      << newline
      << Text(L"Notes Played: ", Gray) << WSTRING(s.notes_user_actually_played) << Text(L" / ", Gray) << WSTRING(s.notes_user_could_have_played) << newline
      << Text(L"Longest Combo: ", Gray) << WSTRING(s.longest_combo) << newline;

}
