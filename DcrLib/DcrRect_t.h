/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// DcrRect_t.h
//
// TODO: DcrWidget_t.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_DCRRECT_T_H
#define Include_DCRRECT_T_H

#include "Dcr.h"
#include "Rect.h"
#include "DdUtil.h"
#include "UiWindow.h"

class CSurface;

/////////////////////////////////////////////////////////////////////////////

class DcrRect_t : 
    public DCR
{

private:

    const Charset_t* m_pCharset;
    const Ui::Window_t& m_window;
    Ui::WidgetId_t   m_widgetId;
    RelativeRect_t   m_Rect;
    std::wstring     m_Text;
    bool             m_checkCaret;
    bool             m_hasCaret;
    COLORREF         m_highBkColor;

public:

    explicit
    DcrRect_t(
        const Ui::Window_t&  window,
              Ui::WidgetId_t widgetId,
        const Charset_t*     pCharset,
              bool           checkCaret = false, // TODO: Flag_t flags
              COLORREF       highBkColor = RGB(0,0,0))
    :
        m_window(window),
        m_widgetId(widgetId),
        m_Rect(NULL),
        m_pCharset(pCharset),
        m_checkCaret(checkCaret),
        m_hasCaret(false),
        m_highBkColor(highBkColor)
    {
        if (NULL == m_pCharset)
        {
            throw std::invalid_argument("DcrRect_t::DcrRect_t()");
        }
    }

    //
    // DCR virtual
    //
    virtual
    bool
    PreTranslateSurface(
        CSurface* pSurface,
        Rect_t&   dcrRect) override
    {
        static const COLORREF kBlack  = RGB(0, 0, 0);
        m_window.GetWidgetRect(m_widgetId, dcrRect);
        if (m_checkCaret)
        {
            static const COLORREF kYellow = RGB(255, 255, 0);
            m_hasCaret = 0 < pSurface->FixColor(dcrRect, kYellow, kYellow, kBlack);
        }
        if (kBlack != m_highBkColor)
        {
            pSurface->FixColor(dcrRect, kBlack, m_highBkColor, kBlack);
        }
#if 1
        static size_t When = 0;
        if (0 == When++)
        {
            pSurface->WriteBMP(L"diag\\DcrRect.bmp", dcrRect);
        }
#endif
        return true;
    }

    virtual
    bool
    TranslateSurface(
        CSurface* pSurface,
        Rect_t&   SurfaceRect) override
    {
        static const size_t MaxTextLength = 255;
        wchar_t Buffer[MaxTextLength];
        // allow bad chars if there is a caret present, so we can get an
        // approximate length of the string, so we know how much we have
        // to delete in order to clear it
        DWORD flags = !m_hasCaret ? 0 : DCR_GETTEXT_ALLOW_BAD;
        HRESULT hr = DCR::GetText(pSurface, &SurfaceRect, Buffer, _countof(Buffer), m_pCharset, flags);
        if (FAILED(hr))
        {
            LogError(L"DcrRect_t::TranslateSurface() failed in GetText()");
            m_Text.clear();
            return false;
        }
        LogInfo(L"DcrRect_t::TranslateSurface(): Text(%ls)", Buffer);
        m_Text.assign(Buffer);
        return true;
    }

    const wstring&
    GetText() const
    {
        return m_Text;
    }

    bool
    GetHasCaret() const
    {
        return m_hasCaret;
    }

private:

    // Explicitly disabled:
    DcrRect_t();
    DcrRect_t(const DcrRect_t&);
    DcrRect_t& operator=(const DcrRect_t&);
};

////////////////////////////////////////////////////////////////////////////////

#endif // Include_DCRRECT_T_H

////////////////////////////////////////////////////////////////////////////////

