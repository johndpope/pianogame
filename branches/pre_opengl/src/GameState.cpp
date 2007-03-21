// Piano Hero
// Copyright (c)2006 Nicholas Piegdon
// See license.txt for license information

#include "GameState.h"
#include "Renderer.h"

void GameState::ChangeState(GameState *new_state)
{
   if (!m_manager) throw GameStateError("Cannot change state if manager not set!");
   m_manager->ChangeState(new_state);
}

int GameState::GetStateWidth() const
{
   if (!m_manager) throw GameStateError("Cannot retrieve state width if manager not set!");
   return m_manager->GetStateWidth();
}

int GameState::GetStateHeight() const
{
   if (!m_manager) throw GameStateError("Cannot retrieve state height if manager not set!");
   return m_manager->GetStateHeight();
}

bool GameState::IsKeyPressed(GameKey key) const
{
   if (!m_manager) throw GameStateError("Cannot determine key presses if manager not set!");
   return m_manager->IsKeyPressed(key);
}

const MouseInfo &GameState::Mouse() const
{
   if (!m_manager) throw GameStateError("Cannot determine mouse input if manager not set!");
   return m_manager->Mouse();
}

void GameState::SetManager(GameStateManager *manager)
{
   if (m_manager) throw GameStateError("State already has a manager!");

   m_manager = manager;
   Init();
}

void GameStateManager::KeyPress(GameKey key)
{
   m_key_presses |= static_cast<unsigned long>(key);
}

bool GameStateManager::IsKeyPressed(GameKey key) const
{
   return ( (m_key_presses & static_cast<unsigned long>(key)) != 0);
}

void GameStateManager::MousePress(MouseButton button)
{
   switch (button)
   {
   case MouseLeft:
      m_mouse.held.left = true;
      m_mouse.released.left = false;
      m_mouse.newPress.left = true;
      break;

   case MouseRight:
      m_mouse.held.right = true;
      m_mouse.released.right = false;
      m_mouse.newPress.right = true;
      break;
   }
}

void GameStateManager::MouseRelease(MouseButton button)
{
   switch (button)
   {
   case MouseLeft:
      m_mouse.held.left = false;
      m_mouse.released.left = true;
      m_mouse.newPress.left = false;
      break;

   case MouseRight:
      m_mouse.held.right = false;
      m_mouse.released.right = true;
      m_mouse.newPress.right = false;
      break;
   }
}

void GameStateManager::MouseMove(int x, int y)
{
   m_mouse.x = x;
   m_mouse.y = y;
}

void GameStateManager::SetInitialState(GameState *first_state)
{
   if (m_current_state) throw GameStateError("Cannot set an initial state because GameStateManager already has a state!");

   first_state->SetManager(this);
   m_current_state = first_state;
}

void GameStateManager::ChangeState(GameState *new_state)
{
   if (!m_current_state) throw GameStateError("Cannot change state without a state!  Use SetInitialState()!");
   if (!new_state) throw GameStateError("Cannot change to a null state!");

   if (!m_inside_update) throw GameStateError("ChangeState must be called from inside another state's "
      "Update() function!  (This is so we can guarantee the ordering of the draw/update calls.)");

   m_next_state = new_state;
}



void GameStateManager::Update()
{
   // Manager's timer grows constantly
   const unsigned long now = timeGetTime();
   const unsigned long delta = now - m_last_milliseconds;
   m_last_milliseconds = now;

   if (m_next_state && m_current_state)
   {
      delete m_current_state;
      m_current_state = 0;

      // We return here to insert a blank frame (that may or may
      // not last a long time) while the next state's Init()
      // and first Update() are being called.
      return;
   }

   if (m_next_state)
   {
      m_current_state = m_next_state;
      m_next_state = 0;

      m_current_state->SetManager(this);
   }

   if (!m_current_state) return;

   m_inside_update = true;

   m_current_state->m_last_delta_milliseconds = delta;
   m_current_state->m_state_milliseconds += delta;
   m_current_state->Update();

   m_inside_update = false;

   // Reset our keypresses for the next frame
   m_key_presses = 0;

   // Reset our mouse clicks for the next frame
   m_mouse.newPress = MouseButtons();
   m_mouse.released = MouseButtons();
}

void GameStateManager::Draw(Renderer &renderer)
{
   if (!m_current_state) return;

   // NOTE: Sweet transition effects are *very* possible here... rendering
   // the previous state *and* the current state during some transition
   // would be really easy.

   // Create a backbuffer to eliminate flicker
   HDC backbuffer_hdc = CreateCompatibleDC(renderer.GetHdc());
   HBITMAP backbuffer = CreateCompatibleBitmap(renderer.GetHdc(), GetStateWidth(), GetStateHeight());
   HGDIOBJ previous_object = SelectObject(backbuffer_hdc, backbuffer);
   SetBkMode(backbuffer_hdc, TRANSPARENT);

   Renderer r(backbuffer_hdc);
   m_current_state->Draw(r);

   // Copy the backbuffer to the screen
   BitBlt(renderer.GetHdc(), 0, 0, GetStateWidth(), GetStateHeight(), backbuffer_hdc, 0, 0, SRCCOPY);

   // Clean up GDI
   SelectObject(backbuffer_hdc, previous_object);
   DeleteObject(backbuffer);
   DeleteDC(backbuffer_hdc);
}
