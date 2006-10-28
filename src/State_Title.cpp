// Piano Hero
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#include "State_Title.h"
#include "State_TrackSelection.h"

#include "version.h"

#include "MenuLayout.h"
#include "Image.h"
#include "registry.h"

#include "libmidi\Midi.h"
#include "libmidi\MidiUtil.h"
#include "libmidi\MidiComm.h"

using namespace std;

void TitleState::Init()
{
   m_back_button = ButtonState(Layout::ScreenMarginX,
      GetStateHeight() - Layout::ScreenMarginX - Layout::ButtonHeight,
      Layout::ButtonWidth, Layout::ButtonHeight);

   m_continue_button = ButtonState(GetStateWidth() - Layout::ScreenMarginX - Layout::ButtonWidth,
      GetStateHeight() - Layout::ScreenMarginX - Layout::ButtonHeight,
      Layout::ButtonWidth, Layout::ButtonHeight);

   // midi_out could be in one of two states right now.  Either we just started
   // and we were passed a null MidiCommOut pointer (and its our responsibility
   // to make a new one) or we've just returned from the track selection state
   // with a valid MidiCommOut object that we constructed ourselves previously.
   if (!m_state.midi_out)
   {
      // Try to find the previously used device
      wstring last_device;
      Registry reg(Registry::CurrentUser, L"Piano Hero");
      reg.Read(L"Last Device", &last_device, L"");

      MidiCommDescriptionList devices = MidiCommOut::GetDeviceList();
      for (size_t i = 0; i < devices.size(); ++i)
      {
         if (devices[i].name == last_device)
         {
            m_state.midi_out = new MidiCommOut(devices[i].id);
         }
      }

      // Next, if we couldn't find a previously used device,
      // use the first one
      if (!m_state.midi_out && devices.size() > 0)
      {
         m_state.midi_out = new MidiCommOut(devices[0].id);
      }

      // Finally, if there simply are no MIDI Output devices,
      // disable the next button.  (The draw function handles
      // alerting the user that they cannot proceed.)
      if (!m_state.midi_out) m_continue_button = ButtonState(-200, -200, 0, 0);
   }
   
   unsigned device_id = 0;
   if (m_state.midi_out)
   {
      device_id = m_state.midi_out->GetDeviceDescription().id;

      m_state.midi_out->Reset();
   }

   // Alright, it's one or the other.  Say you load a big midi.  A reset with
   // a different lead-in/lead-out than the previous reset takes a long time.
   // a Midi object is initialized with lead-in/lead-out of 0.  So, having the
   // title screen use the same means immediate playback.  Now, the track selection
   // and playback states use non-zero lead-in/lead-out, so there is a delay
   // between state changes.
   //
   // If the code below is uncommented, it will *force* an update back down to
   // the default so a device preview will be instantaneous.  Otherwise, if you
   // preview a track in track selection or play a song is the play state, and
   // then return to the main menu and try to do a device preview, there will be
   // a delay.
   //
   // That is the greater of two evils.  You choose a device maybe once.  Forcing
   // an additional delay EVERY time you return to the main menu (which is on the
   // path for quitting the program) is silly.

   // Get the midi ready for immediate playback
   //m_state.midi->Reset(0, 0);
   //PlayDevicePreview(0);

   m_device_tile = DeviceTile((GetStateWidth() - DeviceTileWidth) / 2, 420, device_id);
}

void TitleState::Update()
{
   m_continue_button.Update(MouseInfo(Mouse()));
   m_back_button.Update(MouseInfo(Mouse()));

   MouseInfo device_mouse = MouseInfo(Mouse());
   device_mouse.x -= m_device_tile.GetX();
   device_mouse.y -= m_device_tile.GetY();
   m_device_tile.Update(device_mouse);

   if (m_state.midi_out)
   {
      PlayDevicePreview(static_cast<unsigned long long>(GetDeltaMilliseconds()) * 1000);

      if (m_device_tile.HitPreviewButton())
      {
         m_state.midi_out->Reset();

         if (m_device_tile.IsPreviewOn())
         {
            const unsigned long long PreviewLeadIn  = 0;
            const unsigned long long PreviewLeadOut = 0;
            m_state.midi->Reset(PreviewLeadIn, PreviewLeadOut);

            PlayDevicePreview(0);
         }
      }

      if (m_device_tile.GetOutId() != m_state.midi_out->GetDeviceDescription().id)
      {
         m_state.midi_out->Reset();
         delete m_state.midi_out;
         m_state.midi_out = new MidiCommOut(m_device_tile.GetOutId());

         m_state.midi->Reset(0,0);

         // Write last device to registry
         Registry reg(Registry::CurrentUser, L"Piano Hero");
         reg.Write(L"Last Device", m_state.midi_out->GetDeviceDescription().name);
      }
   }

   if (IsKeyPressed(KeyEscape) || m_back_button.hit)
   {
      delete m_state.midi_out;
      m_state.midi_out = 0;

      PostQuitMessage(0);
      return;
   }

   if (m_state.midi_out && (IsKeyPressed(KeyEnter) || m_continue_button.hit))
   {
      m_state.midi_out->Reset();
      ChangeState(new TrackSelectionState(m_state));
      return;
   }
}

void TitleState::PlayDevicePreview(unsigned long long delta_microseconds)
{
   if (!m_device_tile.IsPreviewOn()) return;

   MidiEventListWithTrackId evs = m_state.midi->Update(delta_microseconds);

   if (!m_state.midi_out) return;

   for (MidiEventListWithTrackId::const_iterator i = evs.begin(); i != evs.end(); ++i)
   {
      m_state.midi_out->Write(i->second);
   }
}

void TitleState::Draw(HDC hdc) const
{
   Image graphics(Image::GetGlobalModuleInstance(), L"BITMAP_LOGO");
   graphics.EnableTransparency();

   int left = GetStateWidth() / 2 - graphics.getWidth() / 2;

   const static int TitleY = 100;
   graphics.beginDrawing(hdc);
   graphics.draw(left, TitleY);
   graphics.endDrawing();

   TextWriter version(left + graphics.getWidth() - 80,
      TitleY + graphics.getHeight(), hdc, false, Layout::SmallFontSize);
   version << Text(L"version ", Gray) << Text(PianoHeroVersionString, Gray);

   Layout::DrawHorizontalRule(hdc, GetStateWidth(), GetStateHeight() - Layout::ScreenMarginY);

   Layout::DrawButton(hdc, m_continue_button, L"Choose Tracks", 15);
   Layout::DrawButton(hdc, m_back_button, L"Exit", 55);

   m_device_tile.Draw(hdc);

   const static int InstructionsY = 234;
   TextWriter instructions(left, InstructionsY, hdc, false, Layout::SmallFontSize);

   const static COLORREF Title = RGB(114, 159, 207);
   const static COLORREF Highlight = RGB(138, 226, 52);

   instructions << Text(L"During Play", Title) << newline
      << Text(L"The ", Gray) << Text(L"up", Highlight) << Text(L" and ", Gray)
      << Text(L"down", Highlight) << Text(L" arrow keys change your view. ", Gray)
      << Text(L"Left", Highlight) << Text(L" and ", Gray) << Text(L"right", Highlight)
      << Text(L" change the song's speed.", Gray) << newline
      << Text(L"Space", Highlight) << Text(L" pauses.  ", Gray) << Text(L"Enter", Highlight)
      << Text(L" will restart the song.  ", Gray)
      << Text(L"Escape", Highlight) << Text(L" returns to track selection.", Gray) << newline

      << newline << newline

      << Text(L"State Of The Code", Title) << newline
      << Text(L"MIDI input isn't there yet.  For now, just set a track or two to ", Gray)
      << Text(L"You Play", Highlight) << Text(L" and follow along", Gray) << newline
      << Text(L"on your own piano.", Gray) << newline;

}
