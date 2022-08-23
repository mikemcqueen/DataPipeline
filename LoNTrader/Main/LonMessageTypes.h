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
    using LonMessageId_t = DP::MessageId_t;

    namespace Message
    {
        constexpr auto MakeId(const unsigned id) {
            //const unsigned max = 0x10000;
            //if consteval {
            //    static_assert(id < max);
            //}
            const auto first = static_cast<unsigned>(DP::Message::Id::Message_First); // TODO or User_First
            return LonMessageId_t(first + id);
        }

        namespace Id
        {
            // TODO: TextTable -> PostedTradesText, ConfirmTradesText, etc.
            // Register the handlers with those IDs; Only send message to the
            // handlers registered for that Id.
            static const auto ScreenShot =  LonMessageId_t(MakeId(0));
            static const auto TextTable =   LonMessageId_t(MakeId(1));
            static const auto RemoveTrade = LonMessageId_t(MakeId(2));
        }

        struct Data_t :
            public DP::Message::Data_t
        {
            Lon::Window::Type_e WindowType;

            Data_t(
                const wchar_t*      _className,
                DP::Stage_t         _stage,
                DP::Message::Type   _messageType,
                DP::MessageId_t     _messageId,
                Lon::Window::Type_e _windowType = Lon::Window::Unknown,
                size_t              _size = sizeof(Data_t))
                :
                DP::Message::Data_t(
                    _stage,
                    _messageId,
                    _size, 
                    _className,
                    _messageType),
                WindowType(_windowType)
            { }

        private:

            Data_t();
        };

    } // Message 

    using LonTransactionId_t = DP::TransactionId_t;

    namespace Transaction
    {
        constexpr auto MakeId(const unsigned id) {
            //const unsigned max = 0x10000;
            //if consteval {
            //    static_assert(id < max);
            //}
            const auto first = static_cast<unsigned>(DP::Transaction::Id::User_First);
            return LonTransactionId_t(first + id);
        }

        namespace Id
        {
            static const auto GetYourCards = LonTransactionId_t(MakeId(0));
            static const auto GatherTrades = LonTransactionId_t(MakeId(1));
            static const auto BuyTrade = LonTransactionId_t(MakeId(2));
            static const auto PostTrade = LonTransactionId_t(MakeId(3));
            static const auto RemoveTrade = LonTransactionId_t(MakeId(4));
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
                DP::MessageId_t     id,
                Lon::Window::Type_e windowType,
                size_t              size = sizeof(Data_t))
                :
                DP::Transaction::Data_t(
                    id,
                    size),
                WindowType(windowType)
            { }
        };
    }
} // Lon

////////////////////////////////////////////////////////////////////////////////

#endif // Include_LONMESSAGETYPES_H

////////////////////////////////////////////////////////////////////////////////
