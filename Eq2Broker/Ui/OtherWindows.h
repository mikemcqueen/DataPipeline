////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// OtherWindows.h
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef INCLUDE_OTHERWINDOWS_H
#define INCLUDE_OTHERWINDOWS_H

#include "UiWindow.h"
#include "Rect.h"
#include "DdUtil.h"

namespace Broker {

  class Eq2LoadingWindow_t : public Ui::Window_t {
  public:
    Eq2LoadingWindow_t();

    const CSurface* GetOriginSurface() const override;
    void GetOriginSearchRect(const CSurface& surface, Rect_t& rect) const override;

  private:
    void loadSurfaces();

    CSurface m_cancelButton;
  };

  class TransitionWindow_t : public Ui::Window_t {
  public:
    TransitionWindow_t();

    const CSurface* GetOriginSurface() const override;
    void GetOriginSearchRect(const CSurface& surface, Rect_t& rect) const override;

  private:
    void loadSurfaces();

    CSurface m_transitionImage;
  };

  class MainChatWindow_t : public Ui::Window_t {
  public:
    MainChatWindow_t();

    const CSurface* GetOriginSurface() const override;
    void GetOriginSearchRect(const CSurface& surface, Rect_t& rect) const override;

  private:
    void loadSurfaces();

    CSurface m_mainChatCaption;
  };

} // Broker

#endif // INCLUDE_OTHERWINDOWS_H