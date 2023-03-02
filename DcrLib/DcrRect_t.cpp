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
#include "Rect.h"
#include "CommonTypes.h"
#include "Timer_t.h"

DcrRect_t::
DcrRect_t(
    int id,
    std::optional<DcrImpl> method,
    bool checkCaret /*= false*/, // TODO: Flag_t flags
    COLORREF highBkColor /*= RGB(0, 0, 0)*/)
    :
    DCR(id, method),
    m_checkCaret(checkCaret),
    m_hasCaret(false),
    m_highBkColor(highBkColor)
{ }

bool
DcrRect_t::
TranslateSurface(
    CSurface* pSurface,
    const Rect_t& rect)
{
    LogInfo(L"DcrRect_t::TranslateSurface (%d)", id());
#if 0
    static std::unordered_map<int, bool> firstTime;
    if (!firstTime.contains(id())) {
        pSurface->WriteBMP(std::format(L"diag\\DcrRect_{}.bmp", id()).c_str(), rect);
        firstTime[id()] = true;
    }
#endif
    Timer_t t("DcrRect_t::GetText");
    text_ = impl().GetText(pSurface, rect);
    t.done();
    LogInfo(L"DcrRect_t(%d): %S", id(), text_.c_str());
    return true;
}
