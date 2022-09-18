/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// BrokerId.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_BROKERID_H
#define Include_BROKERID_H

#include "DpTransaction.h"
#include "DpMessage.h"

/////////////////////////////////////////////////////////////////////////////

namespace Broker
{

template<typename T>
concept underlying_type_int = std::is_same_v<std::underlying_type_t<T>, int>;

int intValue(underlying_type_int auto val) {
    return static_cast<int>(val);
}

    namespace Message
    {

        constexpr auto MakeId(const unsigned id) {
            constexpr auto first = static_cast<unsigned>(DP::Message::Id::Message_First); // TODO or User_First
            return DP::MessageId_t(first + id);
        }

        namespace Id
        {
            constexpr auto Eq2Login = MakeId(0);
            constexpr auto Buy =      MakeId(1);
            constexpr auto Sell =     MakeId(2);
            constexpr auto SalesLog = MakeId(3);
            constexpr auto SetPrice = MakeId(4);

        }

    } // Message

    namespace Transaction
    {
        namespace Id
        {
            constexpr auto MakeId(const unsigned id) -> DP::TransactionId_t {
                const auto first = static_cast<unsigned>(DP::Transaction::Id::User_First);
                return DP::TransactionId_t(first + id);
            }

            constexpr auto SetActiveWindow = MakeId(0);
            constexpr auto BuyTabGetItems = MakeId(1);                          // get buy tab prices
            constexpr auto SellTabGetItems = MakeId(2);                         // get sell tab prices
            constexpr auto GetItemPrices = MakeId(3);                            // get buy tab prices of single item
            constexpr auto GetItemsForSale = MakeId(4);                         // get buy tab prices of all items for sale
            constexpr auto RepriceItems = MakeId(5);                    //5     // reprice owned items 
            constexpr auto Logon = MakeId(6);                   // logon
            constexpr auto SetWidgetText = MakeId(7);                           // set the text of a widget
            constexpr auto BuyItem = MakeId(8);                                 // purchase an item 
            constexpr auto BuySellItems = MakeId(9);                            // combo (SellTabGetItems+itemsToBuySell)->(GetItemPrices)->(RepriceItems)
            constexpr auto OpenBroker = MakeId(10);                      //10    // click the broker npc (or market board)
        }

        namespace Error
        {
            enum : DP::Transaction::Error_t
            {
                TestSucceeded                    = DP::Transaction::Error::User_First,
                InvalidWindow,
                InvalidState,
                InvalidItemName,
                ItemNotFound,
                NoItems,                         // 5
                InvalidLoginHandle,
                BrokerNotFound,
            };
        }
    } // Transaction

} // Broker

/////////////////////////////////////////////////////////////////////////////

#endif // Include_BROKERID_H

/////////////////////////////////////////////////////////////////////////////
