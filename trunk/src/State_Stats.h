// Synthesia
// Copyright (c)2007 Nicholas Piegdon
// See license.txt for license information

#ifndef __STATE_STATS_H
#define __STATE_STATS_H

#include "SharedState.h"
#include "GameState.h"
#include "MenuLayout.h"

class StatsState : public GameState
{
public:
   StatsState(const SharedState &state)
      : m_state(state)
   { }

protected:
   virtual void Init();
   virtual void Update();
   virtual void Draw(Renderer &renderer) const;

private:
   ButtonState m_continue_button;
   ButtonState m_back_button;

   std::wstring m_tooltip;

   SharedState m_state;
};

#endif