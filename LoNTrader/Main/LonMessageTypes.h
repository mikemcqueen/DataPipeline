////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// LonMessageTypes.h
//
////////////////////////////////////////////////////////////////////////////////

#if _MSC_VER >= 1000
#pragma once
#endif

#ifndef Include_LONMESSAGETYPES_H
#define Include_LONMESSAGETYPES_H

#include "LonTypes.h"
#include "DpTransaction.h"


namespace Lon
{
    namespace Message
    {
        namespace Id
        {
            enum : unsigned
            {
                // TODO: TextTable -> PostedTradesText, ConfirmTradesText, etc.
                // Register the handlers with those IDs; Only send message to the
                // handlers registered for that Id.
                ScreenShot,
                TextTable,
                RemoveTrade,
            };
        }

        struct Data_t :
            public DP::Message::Data_t
        {
            Lon::Window::Type_e WindowType;

            Data_t(
                const wchar_t*      Class,
                DP::Stage_t              Stage,
                DP::MessageType_t   MessageType,
                DP::MessageId_t     MessageId,
                Lon::Window::Type_e InitWindowType = Lon::Window::Unknown,
                size_t              Size = sizeof(Data_t))
            :
                DP::Message::Data_t(
                    //Class,
                    Stage,
                    MessageType,
                    DP::MessageId_t(MessageId),
				    Class,
                    Size),
                WindowType(InitWindowType)
            { }

        private:

            Data_t();
        };

    } // Message 

    namespace Transaction
    {
        namespace Id
        {
            enum : unsigned
            {
                GetYourCards,
                GatherTrades,
                BuyTrade,
                PostTrade,
                RemoveTrade
            };
        }

        namespace Error
        {
            enum Type_e
            {
                None = 0,
                TestSucceeded,
                TradeNotFound,
                TradesDontMatch,
                TradeDetailWindowNotOpen,
                BuildingTrade,             // 5
                ConfirmTrade,
                Timeout,
            };
        }
        typedef Error::Type_e Error_e;

        struct Data_t :
            public DP::Transaction::Data_t
        {
            Lon::Window::Type_e WindowType;

            Data_t(
                DP::MessageId_t   Id,
                Lon::Window::Type_e InitWindowType,
                size_t              Size = sizeof(Data_t))
                :
                DP::Transaction::Data_t(
                    Id,
                    Size),
                WindowType(InitWindowType)
            { }
        };
    }
} // Lon

////////////////////////////////////////////////////////////////////////////////

#endif // Include_LONMESSAGETYPES_H

////////////////////////////////////////////////////////////////////////////////
