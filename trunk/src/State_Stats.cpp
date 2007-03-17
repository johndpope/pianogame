// Piano Hero
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#include "State_Stats.h"
#include "State_TrackSelection.h"
#include "State_Playing.h"
#include "Renderer.h"

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

   m_tooltip = L"";
   if (m_back_button.hovering) m_tooltip = L"Return to the track selection screen.";
   if (m_continue_button.hovering) m_tooltip = L"Try this song again with the same settings.";
}

void StatsState::Draw(Renderer &renderer) const
{
   Layout::DrawTitle(renderer.GetHdc(), m_state.song_title);
   Layout::DrawHorizontalRule(renderer.GetHdc(), GetStateWidth(), Layout::ScreenMarginY);
   Layout::DrawHorizontalRule(renderer.GetHdc(), GetStateWidth(), GetStateHeight() - Layout::ScreenMarginY);

   Layout::DrawButton(renderer.GetHdc(), m_continue_button, L"Retry Song", 28);
   Layout::DrawButton(renderer.GetHdc(), m_back_button, L"Track Selection", 13);

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
   
   TextWriter grade_text(left + 110, InstructionsY + 43, renderer.GetHdc(), false, 72);
   grade_text << grade;
   
   int stray_percent = 0;
   if (s.total_notes_user_pressed > 0) stray_percent = static_cast<int>((100.0 * s.stray_notes) / s.total_notes_user_pressed);

   int average_speed = 0;
   if (s.notes_user_could_have_played > 0) average_speed = s.speed_integral / s.notes_user_could_have_played;

   TextWriter score(left, InstructionsY, renderer.GetHdc(), false, Layout::TitleFontSize);
   score << Text(L"Song Statistics", Title) << newline
      << newline 
      << newline 
      << Text(L"Grade: ", Gray) << newline
      << newline
      << newline
      << Text(L"Score: ", Gray) << WSTRING(static_cast<int>(s.score)) << newline
      << Text(L"Average Speed: ", Gray) << WSTRING(average_speed << L" %") << newline
      << newline
      << Text(L"Good Notes: ", Gray) << WSTRING(s.notes_user_actually_played) << Text(L" / ", Gray) << WSTRING(s.notes_user_could_have_played) << Text(WSTRING(L"  (" << static_cast<int>(hit_percent) << L" %" << L")"), Dk_Gray) << newline
      << Text(L"Stray Notes: ", Gray) << WSTRING(s.stray_notes) << Text(WSTRING(L"  (" << stray_percent << L" %" << L")"), Dk_Gray) << newline
      << newline
      << Text(L"Longest Combo: ", Gray) << WSTRING(s.longest_combo) << newline
      << newline;

   TextWriter tooltip(GetStateWidth() / 2, GetStateHeight() - Layout::SmallFontSize - 30, renderer.GetHdc(), true, Layout::ButtonFontSize);
   tooltip << m_tooltip;
}
