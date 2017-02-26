///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TrScrollThumb_t.cpp
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TrScrollThumb_t.h"
#include "PipelineManager.h"
#include "SsWindow.h"
#include "DdUtil.h"
#include "SurfacePoolItem_t.h"

///////////////////////////////////////////////////////////////////////////////

TrScrollThumb_t::
TrScrollThumb_t(
    const Ui::Window_t& MainWindow) 
:
    m_MainWindow(MainWindow)
{
}

///////////////////////////////////////////////////////////////////////////////

TrScrollThumb_t::
~TrScrollThumb_t()
{
}

///////////////////////////////////////////////////////////////////////////////

HRESULT
TrScrollThumb_t::
MessageHandler(
    const DP::Message::Data_t* pData)
{
    using namespace SsWindow;
    if (typeid(*pData) != typeid(Acquire::Data_t))
    {
        return S_FALSE;
    }
    const Acquire::Data_t& SsData = *static_cast<const Acquire::Data_t*>(pData);
    Ui::Window_t& Window = m_MainWindow.GetWindow(SsData.WindowId);
    // Ignore message if window has no scroll flags.
    using namespace Ui::Window;
    Flag_t Flags = Window.GetFlags();
    if (!Flags.TestAny(Flag::ScrollFlags))
    {
        return S_FALSE;
    }
    const CSurface& Surface = *SsData.pPoolItem->get();
    if (Flags.Test(Flag::VerticalScroll))
    {
        Window.UpdateScrollPosition(Ui::Scroll::Bar::Vertical, Surface);
        // , SsData.SurfaceRect);
    }
    if (Flags.Test(Flag::HorizontalScroll))
    {
        Window.UpdateScrollPosition(Ui::Scroll::Bar::Horizontal, Surface);
        // , SsData.SurfaceRect);
    }
    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////
