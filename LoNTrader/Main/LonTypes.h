/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// LonTypes.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_LONTYPES_H
#define Include_LONTYPES_H

/////////////////////////////////////////////////////////////////////////////

namespace Lon
{
    namespace Window
    {
        enum Type_e
        {
            MainWindow                         = 0,

            PostedTradesWindow                 = 100,
            PostedTradesList,                        // list contains viewport and scrollbars
            PostedTradesView,
            PostedTradesVScroll,
            PostedTradesCreateButton,

            PostedTradeDetailWindow            = 200,
            PostedTradeDetailDialog,
            PostedTradeDetailOfferedView,
            PostedTradeDetailOfferedVScroll,
            PostedTradeDetailWantView,
            PostedTradeDetailWantVScroll,
            PostedTradeDetailCancel,
            PostedTradeDetailAccept,

            TradeBuilderWindow                 = 300,
            TradeBuilderCollectionFrame,
            TradeBuilderCollectionTab,
            TradeBuilderSearchEdit,
            TradeBuilderSearchEditClear,
            TradeBuilderYourTableList,
            TradeBuilderYourTableView,
            TradeBuilderYourTableVScroll,
            TradeBuilderYourTableHScroll,
            TradeBuilderTheirTableList,
            TradeBuilderTheirTableView,
            TradeBuilderTheirTableVScroll,
            TradeBuilderTheirTableHScroll,
            TradeBuilderClear,
            TradeBuilderSubmit,
            TradeBuilderExit,

            ConfirmTradeWindow                 = 400,
            ConfirmTradeYouGetView,
            ConfirmTradeYouGetVScroll,
            ConfirmTradeTheyGetView,
            ConfirmTradeTheyGetVScroll,
            ConfirmTradeCancel,
            ConfirmTradeConfirm,

            SystemMessageWindow                = 500,
            SystemMessageClose,

            NetworkStatusWindow                = 600,
            NetworkStatusOk,

            TradeBuilderExitPrompt             = 700,
            TradeBuilderExitPromptYes,

            AcceptTradeWindow                  = 800,
            AcceptTradeYes,
            AcceptTradeNo,

            CannotTradeWindow                  = 900,
            CannotTradeOk,

            DeliveryWindow                     = 1000,
            DeliveryClose,
            DeliveryInfoLabel, // do a dcr on it for trade id?

            JoinGuildWindow                    = 1100,
            JoinGuildNo,

            CancelTradeWindow                  = 1200,
            CancelTradeYes,
            CancelTradeNo,

            Unknown                            = 49152,
        };
    } // Window

    struct ScrollBar_t
    {
        enum Type_e
        {
            UnknownType = -1,
            Vertical,
            Horizontal
        } Type;

        enum ThumbPosition_e
        {
            UnknownPosition = -1,
            Top,
            Bottom,
            Middle
        } Position;

        ScrollBar_t(
            Type_e          InitType = UnknownType,
            ThumbPosition_e InitPosition = UnknownPosition)
        :
            Type(InitType),
            Position(InitPosition)
        { }
    };

} // Lon

/////////////////////////////////////////////////////////////////////////////

#endif // Include_LONTYPES_H

/////////////////////////////////////////////////////////////////////////////
