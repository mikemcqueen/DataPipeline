/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TradeDetailTypes.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_TRADEDETAILTYPES_H
#define Include_TRADEDETAILTYPES_H

#include "LonWindowManager_t.h"
#include "TextTable_t.h"
#include "LonTypes.h"

/////////////////////////////////////////////////////////////////////////////

namespace TradeDetail
{
    static const Lon::Window::Type_e TopWindowType        = Lon::Window::PostedTradeDetailWindow;

    namespace Table
    {

        static const size_t LineCount            = 11;
        static const size_t LineHeight           = 16;
        static const size_t CharHeight           = 12;
        static const size_t CharsPerLine         = 150;
        static const size_t ColumnCount          = 3;
        static const size_t CharColumnWidths []  =  { 5,  5,  75 };
        // Zero as the third column width means "use all the remaining width
        // of the capture rectangle."
        static const size_t PixelColumnWidths[]  = { 60, 60, 0 };

		typedef TextTable_t<
			LineCount,
			CharsPerLine,
			ColumnCount> Text_t;
                    // LineHeight, CharHeight> Text_t; // NOTE removed to make it compile
    }

    namespace Translate
    {
        struct Data_t;
        class Handler_t;
    }

    namespace Interpret
    {
        struct Data_t;
        class Handler_t;
    }

    typedef Lon::WindowManager_t<
                Translate::Handler_t,
                Interpret::Handler_t> ManagerBase_t;
    class Manager_t;

} // TradeDetail

/////////////////////////////////////////////////////////////////////////////

#endif // Include_TRADEDETAILTYPES_H

/////////////////////////////////////////////////////////////////////////////
