/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// ConfirmTradeTypes.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_CONFIRMTRADETYPES_H
#define Include_CONFIRMTRADETYPES_H

#include "LonWindowManager_t.h"
#include "TextTable_t.h"
#include "LonTypes.h"

/////////////////////////////////////////////////////////////////////////////

namespace ConfirmTrade
{
    static const Lon::Window::Type_e TopWindowType        = Lon::Window::ConfirmTradeWindow;

    namespace Table
    {
        static const size_t LineCount                     = 7;
        static const size_t LineHeight                    = 16;
        static const size_t CharHeight                    = 12;
        static const size_t CharsPerLine                  = 80;
        static const size_t ColumnCount                   = 2;

        static const size_t CharColumnWidths []           = { 5,  75 };
        // Zero as the last column width means "use all the remaining width
        // of the capture rectangle."
        static const size_t PixelColumnWidths[]           = { 100, 0 };

        // TODO: Table::Data_t, or Table_t::Data_t
		typedef TextTable_t<
			LineCount,
			CharsPerLine,
			ColumnCount> Text_t; 
           // LineHeight, CharHeight> Text_t;  // NOTE: commented this out to make it compile
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

} // ConfirmTrade

/////////////////////////////////////////////////////////////////////////////

#endif // Include_CONFIRMTRADETYPES_H

/////////////////////////////////////////////////////////////////////////////
