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

const static wstring OutputDeviceKey = L"Last Output Device";
const static wstring OutputKeySpecialDisabled = L"[no output device]";

const static wstring InputDeviceKey = L"Last Input Device";
const static wstring InputKeySpecialDisabled = L"[no input device]";

void TitleState::Init()
{
   m_back_button = ButtonState(Layout::ScreenMarginX,
      GetStateHeight() - Layout::ScreenMarginX - Layout::ButtonHeight,
      Layout::ButtonWidth, Layout::ButtonHeight);

   m_continue_button = ButtonState(GetStateWidth() - Layout::ScreenMarginX - Layout::ButtonWidth,
      GetStateHeight() - Layout::ScreenMarginX - Layout::ButtonHeight,
      Layout::ButtonWidth, Layout::ButtonHeight);

   Registry reg(Registry::CurrentUser, L"Piano Hero");

   wstring last_output_device;
   reg.Read(OutputDeviceKey, &last_output_device, L"");

   wstring last_input_device;
   reg.Read(InputDeviceKey, &last_input_device, L"");

   // midi_out could be in one of three states right now:
   //    1. We just started and were passed a null MidiCommOut pointer
   // Or, we're returning from track selection either with:
   //    2. a null MidiCommOut because the user didn't want any output, or
   //    3. a valid MidiCommOut we constructed previously.
   if (!m_state.midi_out)
   {
      // Try to find the previously used device
      MidiCommDescriptionList devices = MidiCommOut::GetDeviceList();
      for (size_t i = 0; i < devices.size(); ++i)
      {
         if (devices[i].name == last_output_device)
         {
            m_state.midi_out = new MidiCommOut(devices[i].id);
         }
      }

      // Next, if we couldn't find a previously used device,
      // use the first one
      if (last_output_device != OutputKeySpecialDisabled && !m_state.midi_out && devices.size() > 0)
      {
         m_state.midi_out = new MidiCommOut(devices[0].id);
      }
   }

   if (!m_state.midi_in)
   {
      // Try to find the previously used device
      MidiCommDescriptionList devices = MidiCommIn::GetDeviceList();
      for (size_t i = 0; i < devices.size(); ++i)
      {
         if (devices[i].name == last_input_device)
         {
            m_state.midi_in = new MidiCommIn(devices[i].id);
         }
      }

      // If we couldn't find the previously used device,
      // disabling by default (i.e. leaving it null) is
      // completely acceptable.
   }

   int output_device_id = 0;
   if (last_output_device == OutputKeySpecialDisabled) output_device_id = -1;

   if (m_state.midi_out)
   {
      output_device_id = m_state.midi_out->GetDeviceDescription().id;
      m_state.midi_out->Reset();
   }

   int input_device_id = -1;
   if (m_state.midi_in)
   {
      input_device_id = m_state.midi_in->GetDeviceDescription().id;
      m_state.midi_in->Reset();
   }

   m_output_tile = DeviceTile((GetStateWidth() - DeviceTileWidth) / 2, 420, DeviceTileOutput, output_device_id);
   m_input_tile  = DeviceTile((GetStateWidth() - DeviceTileWidth) / 2, 520, DeviceTileInput,  input_device_id);
}

void TitleState::Update()
{
   m_continue_button.Update(MouseInfo(Mouse()));
   m_back_button.Update(MouseInfo(Mouse()));

   MouseInfo output_mouse = MouseInfo(Mouse());
   output_mouse.x -= m_output_tile.GetX();
   output_mouse.y -= m_output_tile.GetY();
   m_output_tile.Update(output_mouse);

   MouseInfo input_mouse = MouseInfo(Mouse());
   input_mouse.x -= m_input_tile.GetX();
   input_mouse.y -= m_input_tile.GetY();
   m_input_tile.Update(input_mouse);

   // Check to see if we need to switch to a newly selected output device
   int output_id = m_output_tile.GetDeviceId();
   if (!m_state.midi_out || output_id != m_state.midi_out->GetDeviceDescription().id)
   {
      if (m_state.midi_out) m_state.midi_out->Reset();

      delete m_state.midi_out;
      m_state.midi_out = 0;

      // Write last device to registry
      Registry reg(Registry::CurrentUser, L"Piano Hero");

      if (output_id >= 0)
      {
         m_state.midi_out = new MidiCommOut(output_id);
         m_state.midi->Reset(0,0);

         reg.Write(OutputDeviceKey, m_state.midi_out->GetDeviceDescription().name);
      }
      else
      {
         reg.Write(OutputDeviceKey, OutputKeySpecialDisabled);
      }
   }

   if (m_state.midi_out)
   {
      PlayDevicePreview(static_cast<unsigned long long>(GetDeltaMilliseconds()) * 1000);

      if (m_output_tile.HitPreviewButton())
      {
         m_state.midi_out->Reset();

         if (m_output_tile.IsPreviewOn())
         {
            const unsigned long long PreviewLeadIn  = 0;
            const unsigned long long PreviewLeadOut = 0;
            m_state.midi->Reset(PreviewLeadIn, PreviewLeadOut);

            PlayDevicePreview(0);
         }
      }
   }


   int input_id = m_input_tile.GetDeviceId();
   if (!m_state.midi_in || input_id != m_state.midi_in->GetDeviceDescription().id)
   {
      if (m_state.midi_in) m_state.midi_in->Reset();
      m_last_input_note_name = "";

      delete m_state.midi_in;
      m_state.midi_in = 0;

      // Write last device to registry 
      Registry reg(Registry::CurrentUser, L"Piano Hero");

      if (input_id >= 0)
      {
         m_state.midi_in = new MidiCommIn(input_id);

         reg.Write(InputDeviceKey, m_state.midi_in->GetDeviceDescription().name);
      }
      else
      {
         reg.Write(InputDeviceKey, InputKeySpecialDisabled);
      }
   }

   if (m_state.midi_in && m_input_tile.IsPreviewOn())
   {
      // Read note events to display on screen
      while (m_state.midi_in->KeepReading())
      {
         MidiEvent ev = m_state.midi_in->Read();
         if (ev.Type() == MidiEventType_NoteOff || ev.Type() == MidiEventType_NoteOn)
         {
            string note = MidiEvent::NoteName(ev.NoteNumber());

            if (ev.Type() == MidiEventType_NoteOn && ev.NoteVelocity() > 0)
            {
               m_last_input_note_name = note;
            }
            else
            {
               if (note == m_last_input_note_name) m_last_input_note_name = "";
            }
         }
      }
   }
   else
   {
      m_last_input_note_name = "";
   }


   if (IsKeyPressed(KeyEscape) || m_back_button.hit)
   {
      delete m_state.midi_out;
      m_state.midi_out = 0;

      delete m_state.midi_in;
      m_state.midi_in = 0;

      PostQuitMessage(0);
      return;
   }

   if (IsKeyPressed(KeyEnter) || m_continue_button.hit)
   {
      if (m_state.midi_out) m_state.midi_out->Reset();
      if (m_state.midi_in) m_state.midi_in->Reset();

      ChangeState(new TrackSelectionState(m_state));
      return;
   }
}

void TitleState::PlayDevicePreview(unsigned long long delta_microseconds)
{
   if (!m_output_tile.IsPreviewOn()) return;

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

   m_output_tile.Draw(hdc);

   m_input_tile.Draw(hdc);

   TextWriter last_note(m_input_tile.GetX() + DeviceTileWidth + 20, m_input_tile.GetY(), hdc, false, Layout::SmallFontSize);
   Widen<wchar_t> w;
   last_note << w(m_last_input_note_name);

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
