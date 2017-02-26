/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TradeBuilderTypes.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_TRADEBUILDERTYPES_H
#define Include_TRADEBUILDERTYPES_H

#include "LonWindowManager_t.h"
#include "TextTable_t.h"
#include "LonTypes.h"

/////////////////////////////////////////////////////////////////////////////

namespace TradeBuilder
{
    static const Lon::Window::Type_e TopWindowType = Lon::Window::TradeBuilderWindow;

    namespace Table
    {
        static const size_t LineHeightPixels = 30;
        static const size_t CharHeightPixels = 12;
        static const size_t LineCount        = 12;

        // TODO: should be able to *static*ally calculate/verify this with template
        static const size_t CharsPerLine     = 140; 

        static const size_t TheirColumnCount = 4;
        static const size_t TheirPixelColumnWidths[TheirColumnCount] = { 80, 80, 70, 260 };
        static const size_t TheirCharColumnWidths[TheirColumnCount]  = { 10, 10, 10, 110 };

#if FIRST_5_COLUMNS
        static const size_t YourColumnCount  = 5;
        static const size_t YourPixelColumnWidths[] =  { 80, 70, 80, 80, 260 };
        static const size_t YourCharColumnWidths[] =   { 10, 10, 10, 10, 70 };
#else
        static const size_t YourColumnCount  = 7;
        static const size_t YourPixelColumnWidths[YourColumnCount] =  { 80, 80, 260, 70, 70, 50, 75 };
        static const size_t YourCharColumnWidths[YourColumnCount]  =  { 10, 10, 70, 20, 10, 10, 10 };
#endif

        static const size_t MaxColumnCount   = max(YourColumnCount, TheirColumnCount);

		typedef TextTable_t<LineCount, CharsPerLine, MaxColumnCount> Text_t;
                        //    LineHeightPixels, CharHeightPixels> Text_t; // NOTE removed to fix compile error
    }

    // NOTE: Order of Collection_e is important. It matches the tab order
    //       on the Trade Builder window.
    namespace Collection
    {
        enum Type_e
        {
            Yours,
            Theirs
        };
    }
    typedef Collection::Type_e Collection_e;

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

} // TradeBuilder

/////////////////////////////////////////////////////////////////////////////

#endif // Include_TRADEBUILDERTYPES_H

/////////////////////////////////////////////////////////////////////////////
