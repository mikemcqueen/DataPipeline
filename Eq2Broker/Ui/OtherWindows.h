////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// OtherWindows.h
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "UiWindow.h"
#include "Rect.h"
#include "DdUtil.h"

namespace Broker
{

////////////////////////////////////////////////////////////////////////////////

class Eq2LoadingWindow_t :
    public Ui::Window_t
{
private:

    CSurface m_cancelButton;

public:

    Eq2LoadingWindow_t();

    virtual
    const CSurface*
    GetOriginSurface() const override;

    virtual
    void
    GetOriginSearchRect(
        const CSurface& surface,
              Rect_t&   rect) const override;

private:

    void
    loadSurfaces();
};

////////////////////////////////////////////////////////////////////////////////

class TransitionWindow_t :
    public Ui::Window_t
{
private:

    CSurface m_transitionImage;

public:

    TransitionWindow_t();

    virtual
    const CSurface*
    GetOriginSurface() const override;

    virtual
    void
    GetOriginSearchRect(
        const CSurface& surface,
              Rect_t&   rect) const override;

private:

    void
    loadSurfaces();
};

////////////////////////////////////////////////////////////////////////////////

class MainChatWindow_t :
    public Ui::Window_t
{
private:

    CSurface m_mainChatCaption;

public:

    MainChatWindow_t();

    virtual
    const CSurface*
    GetOriginSurface() const override;

    virtual
    void
    GetOriginSearchRect(
        const CSurface& surface,
              Rect_t&   rect) const override;

private:

    void
    loadSurfaces();
};

////////////////////////////////////////////////////////////////////////////////

} // Broker
