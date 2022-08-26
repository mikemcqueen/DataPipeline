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
    namespace Message
    {

        constexpr auto MakeId(const unsigned id) {
            //const unsigned max = 0x10000;
            //if consteval {
            //    static_assert(id < max);
            //}
            constexpr auto first = static_cast<unsigned>(DP::Message::Id::Message_First); // TODO or User_First
            return DP::MessageId_t(first + id);
        }

        namespace Id
        {
            /*
            enum : DP::MessageId_t
            {
                First                              = DP::Message::Id::User_First,
                Eq2Login                           = First,
                Buy,
                Sell,
                SalesLog,
                SetPrice,
            };
            */

            //constexpr auto First = DP::Message::Id::User_First
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
            enum : DP::TransactionId_t
            {
                First                            = DP::Transaction::Id::User_First,
                SetActiveWindow                  = First,
                BuyTabGetItems,                          // get buy tab prices
                SellTabGetItems,                         // get sell tab prices
                GetItemPrices,                           // get buy tab prices of single item
                GetItemsForSale,                         // get buy tab prices of all items for sale
                RepriceItems,                    //5     // reprice owned items 
                Logon,                                   // logon
                SetWidgetText,                           // set the text of a widget
                BuyItem,                                 // purchase an item 
                BuySellItems,                            // combo (SellTabGetItems+itemsToBuySell)->(GetItemPrices)->(RepriceItems)
                OpenBroker,                      //10    // click the broker npc (or market board)
            };
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
