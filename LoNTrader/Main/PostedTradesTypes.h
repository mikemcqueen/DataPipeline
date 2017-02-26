///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// PostedTradesTypes.h
//
///////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_POSTEDTRADESTYPES_H
#define Include_POSTEDTRADESTYPES_H

///////////////////////////////////////////////////////////////////////////////

#include "TiBase_t.h"
#include "LonWindowManager_t.h"
#include "DpEvent.h"
#include "EventTypes.h"

///////////////////////////////////////////////////////////////////////////////

class Trade_t;

namespace PostedTrades
{
    static const Lon::Window::Type_e TopWindowType = Lon::Window::PostedTradesWindow;

    namespace Table
    {
        static const size_t LineCount            = 24;
        static const size_t LineHeight           = 16;
        static const size_t CharHeight           = 12;
        static const size_t CharsPerLine         = 250;
        static const size_t ColumnCount          = 5;
        static const size_t CharColumnWidths [ColumnCount] = { 20,  30,  75,  75,  50 };
        static const size_t PixelColumnWidths[ColumnCount] = { 56, 124, 270, 268, 190 };

		typedef TextTableData_t<
			LineCount,
			CharsPerLine,
			ColumnCount> TextData_t;
             //       LineHeight, CharHeight>          TextData_t;

		typedef TextTable_t<
			Table::LineCount,
			Table::CharsPerLine,
			Table::ColumnCount> Text_t;
                    // Table::LineHeight,Table::CharHeight>   Text_t;

    }
    struct EventAddTrade_t
    {
        struct Data_t :
            public DP::Event::Data_t
        {
            Trade_t& Trade;

            Data_t(
                Trade_t& InitTrade,
                size_t   InitSize = sizeof(Data_t))            
            :
                DP::Event::Data_t(
                    DP::Stage::Analyze,
                    Lon::Event::Id::AddTrade,
                    0,
                    InitSize),
                Trade(InitTrade)
            { }
        } m_Data;

        EventAddTrade_t(
            Trade_t& Trade)
        :
            m_Data(Trade)
        { }

    private:

        EventAddTrade_t();
    };

    /////////////////////////////////////////////////////////////////////////////

    namespace Translate
    {

        class Handler_t;

        struct Data_t :
            public Lon::Message::Data_t
        {
            Table::Text_t     Text;
            RECT              Rect;

        public:

            void
            Initialize(
                const Handler_t* pClass);

        private:

            Data_t();
        };

    } // Translate

    /////////////////////////////////////////////////////////////////////////////

    namespace Interpret
    {
        struct Data_t;
        class Handler_t;

    } // Interpret

    typedef Lon::WindowManager_t<
                Translate::Handler_t,
                Interpret::Handler_t> ManagerBase_t;
    class Manager_t;

} // PostedTrades

///////////////////////////////////////////////////////////////////////////////

#endif // Include_POSTEDTRADESTYPES_H

///////////////////////////////////////////////////////////////////////////////
