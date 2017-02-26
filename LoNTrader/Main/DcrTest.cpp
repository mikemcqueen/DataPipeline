/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// DcrTest.cpp
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PipelineManager.h"
#include "Log_t.h"
#include "Log.h"
#include "DdUtil.h"
#include "Charset_t.h"
#include "Dcr.h"
#include "SurfacePoolItem_t.h"
#include "Macros.h"
#include "SsTrades_t.h"
#include "DcrTrades_t.h"
#include "TiPostedTrades.h"
#include "DcrPostedTrades.h"
#include "TradeManager_t.h"
#include "LonCardSet_t.h"
#include "LonCard_t.h"
#include "TrScrollThumb_t.h"
#include "LonTrader_t.h"

/////////////////////////////////////////////////////////////////////////////

extern CDisplay *g_pDisplay;
// get rid of this
extern bool g_bWriteBmps;

/////////////////////////////////////////////////////////////////////////////

bool
//LonTrader_t::
DcrTest(
    wchar_t* arg,
    bool     bTable)
{
#if 0
arg; bTable;
#else
    std::auto_ptr<CSurface> spSurface(new CSurface());
    HRESULT hr = g_pDisplay->CreateSurfaceFromBitmap(spSurface.get(), arg);
    if (FAILED(hr))
    {
        LogError(L"CreateSurfaceFromBitmap failed ('%ls', 0x%08x)", arg, hr);
		return false;
    }
    CSurface& surf = *spSurface.get();
    RECT rcClient;
    surf.GetClientRect(&rcClient);

    LonWindow_t Window;
    PostedTrades::Translate::Handler_t TrPostedTrades;
    // Create the static charsets.
    bool bInit = TrPostedTrades.Initialize(L"test");
    ASSERT(bInit);

    g_bWriteBmps = true;

    RECT rc = {0};
    rc.right = surf.GetWidth();
    rc.bottom = surf.GetHeight();
    if (!bTable)
    {
        wchar_t buf[256] = L"";
        TrPostedTrades.GetDcr().GetText(&surf, &rc, buf, _countof(buf), NULL,
                                        TrPostedTrades.GetDcr().GetCharsets());
        wprintf(L"Chars: '%ls'\n", buf);
        return true;
    }
    else
    {
#if 1
		Rect_t r(rc); // NOTE added to get it compiling
		if (!TrPostedTrades.GetDcr().PreTranslateSurface(&surf, r) ||
            !TrPostedTrades.GetDcr().TranslateSurface(&surf, r))
            return false;

        TrPostedTrades.GetText().Dump(NULL, true);
#else
        LogAlways(L"Not implemented");
#endif
    }
#endif
    return true;
}
