/////////////////////////////////////////////////////////////////////////////
//
// DcrRect_t.cpp
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DcrRect_t.h"
#include "DdUtil.h"
#include "Log.h"
#include "Macros.h"
//#include "Charset_t.h"
#include "Rect.h"
#include "CommonTypes.h"

/////////////////////////////////////////////////////////////////////////////

DcrRect_t::
DcrRect_t(
    //const Ui::Window_t& window,
    int id,
    const Charset_t* pCharset /*= nullptr*/,
    bool checkCaret /*= false*/, // TODO: Flag_t flags
    COLORREF highBkColor /*= RGB(0, 0, 0)*/)
    :
    DCR(id),
    m_pCharset(pCharset),
    m_Rect(nullptr),
    m_checkCaret(checkCaret),
    m_hasCaret(false),
    m_highBkColor(highBkColor)
{ }

/////////////////////////////////////////////////////////////////////////////

#if 0
bool
DcrRect_t::
PreTranslateSurface(
    CSurface* pSurface,
    Rect_t* pRect)
{
    constexpr COLORREF kBlack = RGB(0, 0, 0);
    m_window.GetWidgetRect(m_widgetId, pRect);
    if (m_checkCaret) {
        constexpr COLORREF kYellow = RGB(255, 255, 0);
        m_hasCaret = 0 < pSurface->FixColor(*pRect, kYellow, kYellow, kBlack);
    }
    /*
    if (kBlack != m_highBkColor) {
        pSurface->FixColor(dcrRect, kBlack, m_highBkColor, kBlack);
    }
    */
    return true;
}
#endif

/////////////////////////////////////////////////////////////////////////////

bool
DcrRect_t::
TranslateSurface(
    CSurface* pSurface,
    const Rect_t& rect)
{
    auto id = GetId();
    LogInfo(L"DcrRect_t::TranslateSurface (%d)", id);
#if 1
    static std::unordered_map<int, bool> firstTime;
    if (!firstTime.contains(id)) {
        pSurface->WriteBMP(std::format(L"diag\\DcrRect_{}.bmp", id).c_str(), rect);
        firstTime[id] = true;
    }
#endif
    if (!UsingTesseract()) {
        return TranslateRect(pSurface, rect);
    }
    text_ = TesseractGetText(pSurface, rect);
    LogInfo(L"DcrRect_t(%d): %S", id, text_.c_str());
    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool
DcrRect_t::
TranslateRect(
    CSurface* pSurface,
    const Rect_t& rect)
{
    constexpr auto MaxTextLength = 255;
    wchar_t buffer[MaxTextLength];
    // allow bad chars if there is a caret present, so we can get an
    // approximate length of the string, so we know how much we have
    // to delete in order to clear it
    DWORD flags = !m_hasCaret ? 0 : DCR_GETTEXT_ALLOW_BAD;
    HRESULT hr = DCR::GetText(pSurface, &rect, buffer, _countof(buffer), m_pCharset, flags);
    if (FAILED(hr)) {
        LogError(L"DcrRect_t::TranslateSurface() failed in GetText()");
        text_.clear();
        return false;
    }
    LogInfo(L"DcrRect_t::TranslateSurface(): Text(%s)", buffer);

    // WCHAR/char changes. old and unused. alert usage via exception.
    //text_.assign(buffer);
    //return true;
    throw std::runtime_error("old code path");
}

/////////////////////////////////////////////////////////////////////////////

bool
DcrRect_t::
TesseractTranslateRect(
    CSurface* pSurface,
    const Rect_t& rect)
{
    rect; pSurface;
    return true;
}

/////////////////////////////////////////////////////////////////////////////
