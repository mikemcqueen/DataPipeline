////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// TxRepriceItems.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TxRepriceItems.h"
#include "PipelineManager.h"
#include "TransactionManager.h"
#include "BrokerBuy.h"
#include "BrokerBuyText.h"
#include "BrokerSell.h"
#include "SetPrice.h"
#include "BrokerId.h"
#include "TiBase_t.h"
#include "MainWindow_t.h"
#include "UiEvent.h"
#include "TiBrokerSell.h"
#include "BrokerWindow.h"
#include "TxBuyTabGetItems.h"
#include "Eq2Broker_t.h" // getcoinstring
#include "DbItems_t.h"

#define EXTRALOG 1

////////////////////////////////////////////////////////////////////////////////

namespace Broker
{
namespace Transaction
{
namespace RepriceItems
{ 

////////////////////////////////////////////////////////////////////////////////

void
Data_t::
Init()
{
    sellItemNames.clear();
    ItemDataMap_t::const_iterator it = brokerItemData.begin();
    for (; brokerItemData.end() != it; ++it)
    {
        sellItemNames.insert(it->first);
    }
    sellItemName.clear();
    hasScrolledToTop = false;
    price = 0;
}

////////////////////////////////////////////////////////////////////////////////

Handler_t::
Handler_t(
    MainWindow_t& mainWindow)
:
    m_mainWindow(mainWindow)
{
}

/////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
ExecuteTransaction(
    DP::Transaction::Data_t& txData)
{
    LogInfo(L"TxRepriceItems::ExecuteTransaction(%d)", txData.Id);
    Data_t& repriceData = static_cast<Data_t&>(txData);
    repriceData.Init();
    if (!repriceData.sellItemNames.empty())
    {
        repriceData.SetState(State::First);
    }
    else
    {
        repriceData.Complete();
    }
/*
    DP::Transaction::Data_t* pTxGetItems = new SellTabGetItems::Data_t(repriceData.myItemData);
    GetTransactionManager().ExecuteTransaction(pTxGetItems, true);
*/
    return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
ResumeTransaction(
          DP::Transaction::Data_t& data,
    const DP::Transaction::Data_t* pPrevTxData)
{
pPrevTxData;
    LogAlways(L"TxRepriceItems::ResumeTransaction()");
    Data_t& txData = static_cast<Data_t&>(data);
    if (DP::Transaction::Error::None == txData.Error)
    {
        txData.NextState();
    }
    else
    {
        txData.Complete(txData.Error);
    }
    return S_OK;
}

////////////////////////////////////////////////////////////////////////////////


HRESULT
Handler_t::
OnTransactionComplete(
    const DP::Transaction::Data_t&)
{
    LogInfo(L"TxRepriceItems::TransactionComplete()");
    return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT
Handler_t::
MessageHandler(
    const DP::Message::Data_t* pMessage)
{
    LogInfo(L"TxRepriceItems::MessageHandler()");
    DP::TransactionManager_t::AutoRelease_t tm(GetTransactionManager().Acquire());
    if (nullptr == tm.get())
    {
        throw logic_error("TxRepriceItems::MessageHandler(): No transaction active");
    }
    Data_t& txData = static_cast<Data_t&>(*tm.get());
    if (!ValidateMessage(*pMessage, txData))
    {
        return S_FALSE;
    }
    static const size_t kStateTimeoutThreshold = 30;
    if (txData.IncStateTimeout() > kStateTimeoutThreshold)
    {
        if (State::First == txData.GetState())
        {
            txData.Complete(DP::Transaction::Error::Timeout);
        }
        else
        {
            LogWarning(L"TxRepricesItems Timeout - going to previous state");
            txData.PrevState();
        }
        return S_OK;
    }
    Ui::Window_t& Window = m_mainWindow.GetWindow(m_mainWindow.GetMessageWindowId(pMessage->Id));
    const Sell::Translate::Data_t&
        SellMessage = static_cast<const Sell::Translate::Data_t&>(*pMessage);
    const SetPrice::Translate::Data_t&
        SetPriceMessage = static_cast<const SetPrice::Translate::Data_t&>(*pMessage);

    switch (txData.GetState())
    {
    case State::SelectItem:
        if (SelectSellItem(SellMessage, txData,
                           static_cast<const TableWindow_t&>(Window)))
        {
            txData.NextState();
        }
        else
        {
            LogWarning(L"TxRepriceItems::SelectItem() SelectSellItem() failed, "
                       L"sellItemNames.size(%d)", txData.sellItemNames.size());
        }
        break;
    case State::ValidateSelected:
        {
            size_t row;
            if (GetSellItemRow(SellMessage, txData, row))
            {
                ++row; // coz SellMessage.selectedRow is 1-based
                if (SellMessage.selectedRow == row)
                {
                    txData.NextState();
                }
            }
            else
            {
                txData.PrevState();
            }
        }
        break;
    case State::ClickSetPrice:
        {
            txData.price = CalcPrice(txData);
            Window.ClickWidget(Sell::Widget::Id::SetPriceButton);
            txData.NextState();
        }
        break;
    case State::ValidatePopup:
        {
            txData.NextState();
        }
        break;
    case State::EnterPrice:
        if (SetItemPrice(SetPriceMessage, txData, Window))
        {
            txData.NextState();
        }
        break;
    case State::ValidatePopupPrice:
        if (ValidatePrice(SetPriceMessage, txData))
        {
            txData.NextState();
        }
        else
        {
            // TODO: probably need a 'clear price' state here
            txData.PrevState();
        }
        break;

    case State::ClosePopup:
        {
            Window.ClickWidget(SetPrice::Widget::Id::OkButton);
            txData.NextState();
        }
        break;

    case State::ValidateSellTab:
        {
            txData.NextState();
        }
        break;

    case State::ListItem:
        ListItem(SellMessage, txData, Window);
        if (NextItem(SellMessage, txData))
        {
            txData.SetState(State::SelectItem);
        }
        break;

    default:
        LogError(L"TxRepriceItems::MessageHandler(): Invalid state (%x)",
                 txData.GetState());
        throw std::logic_error("TxRepriceItems::MessageHandler() invalid state");
    }
    return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

size_t
Handler_t::
CalcPrice(
    const Data_t& txData) const
{
    if (!txData.sellItemName.empty())
    {
        ItemDataMap_t::const_iterator it = txData.brokerItemData.find(txData.sellItemName);
        if (txData.brokerItemData.end() != it)
        {
            const PriceCountMap_t& priceMap = it->second;
            size_t price = 0;
            if (!priceMap.empty())
            {
                const double commission = 0.20;
                const double divisor = 1.0 + commission;
                price = priceMap.begin()->first - 1;
                price = (size_t)((double)price / divisor); // 1.2 = 1.0 + commission
            }
            return price;
        }
        // this could happen if brokerItemData is stale
        throw logic_error("TxRepriceItems::CalcPrice() end() == brokerItemData.find(sellItemName)");
    }
    throw logic_error("TxRepriceItems::CalcPrice() sellItemName.empty()");
}

////////////////////////////////////////////////////////////////////////////////

bool
Handler_t::
ValidateMessage(
    const DP::Message::Data_t& Message,
          Data_t&              txData) const
{
    static const
    struct StateInfo_t
    {
        DP::Transaction::State_t State;
        DP::MessageId_t          MessageId;
        bool                     bEnforce; // i.e., throw an error if message isn't this type?
    } StateInfo[] =
    {
        State::SelectItem,               Message::Id::Sell,      true,
        State::ValidateSelected,         Message::Id::Sell,      true,
        State::ClickSetPrice,            Message::Id::Sell,      true,
        State::ValidatePopup,            Message::Id::SetPrice,  false,
        State::EnterPrice,               Message::Id::SetPrice,  true,
        State::ValidatePopupPrice,       Message::Id::SetPrice,  true,
        State::ClosePopup,               Message::Id::SetPrice,  true,
        State::ValidateSellTab,          Message::Id::Sell,      false,
        State::ListItem,                 Message::Id::Sell,      true
    };

    for (size_t Index = 0; Index < _countof(StateInfo); ++Index)
    {
        const StateInfo_t& Info = StateInfo[Index];
        if (Info.State == txData.GetState())
        {
            if (Info.MessageId == Message.Id)
            {
                return true;
            }
            else if (Info.bEnforce)
            {
                LogError(L"RepriceItems::ValidateMessage(): MessageId (%x) ExpectedId (%x)",
                         Message.Id, Info.MessageId);
                txData.Complete(Error::InvalidWindow);
            }
            return false;
        }
    }
    LogError(L"RepriceItems::ValidateMessage(): Invalid State(%x)", txData.GetState());
    txData.Complete(Error::InvalidState);
    return false;
}

////////////////////////////////////////////////////////////////////////////////

bool
Handler_t::
SelectSellItem(
    const Sell::Translate::Data_t& Message,
          Data_t&                  txData,
    const TableWindow_t&           Window) const
{
    size_t Row;
    if (GetSellItemRow(Message, txData, Row))
    {
        return Window.ClickRow(Row);
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////

bool
Handler_t::
GetSellItemRow(
    const Sell::Translate::Data_t& Message,
          Data_t&                  txData,
          size_t&                  OutRow) const
{
    if (txData.sellItemName.empty())
    {
        return GetAnySellItemRow(Message, txData, OutRow);
    }
    else
    {
        const Sell::Text_t& text = Message.Text;
        const size_t itemNameOffset = text.GetColumnOffset(Sell::Table::ItemNameColumn);
        for (size_t Row = 0; Row < text.GetEndRow(); ++Row)
        {
            const wstring itemName(&text.GetRow(Row)[itemNameOffset]);
            if (itemName == txData.sellItemName)
            {
                OutRow = Row;
                return true;
            }
        }
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////

bool
Handler_t::
GetAnySellItemRow(
    const Sell::Translate::Data_t& Message,
          Data_t&                  txData,
          size_t&                  OutRow) const
{
    const Sell::Text_t& text = Message.Text;
    // find a visible item name that is in itemNames set
    for (size_t Row = 0; Row < Message.Text.GetEndRow(); ++Row)
    {
        wstring itemName(&text.GetRow(Row)[text.GetColumnOffset(Sell::Table::ItemNameColumn)]);
        if (txData.sellItemNames.end() != txData.sellItemNames.find(itemName))
        {
            txData.sellItemName = itemName;
            OutRow = Row;
            return true;
        }
    }
    using Sell::Interpret::Handler_t;
    if (!txData.hasScrolledToTop && !Handler_t::IsScrolledToTop(Message))
    {
        ScrollSell(Ui::Scroll::Direction::Up);
    }
    else
    {
        txData.hasScrolledToTop = true;
        if (!Handler_t::IsScrolledToBottom(Message))
        {
            ScrollSell(Ui::Scroll::Direction::Down);
        }
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////

void
Handler_t::
ScrollSell(
    Ui::Scroll::Direction_t direction) const
{
    using namespace Ui::Scroll::Direction;
    LogAlways(L"TxRepriceItems::Scroll(%s)", (Up == direction) ? L"Up" : L"Down");
    m_mainWindow.GetWindow(Window::Id::BrokerSellTab).Scroll(direction);
}

////////////////////////////////////////////////////////////////////////////////

bool
Handler_t::
SetItemPrice(
    const SetPrice::Translate::Data_t& Message,
          Data_t&                      txData,
    const Ui::Window_t&                Window) const
{
txData; Message;
    size_t Price = txData.price;
    LogAlways(L"TxRepriceItems::SetItemPrice(): Price (%d)", Price);

#if 1
    Window;
    Ui::Event::SendChars::Data_t sendChars(GetCoinString(Price));
    GetPipelineManager().SendEvent(sendChars);
#else
    const size_t Platinum = Price / 100;
    const size_t Gold = Price % 100;

GetPipelineManager().StopAcquiring();
    using namespace SetPrice::Widget::Id;
    Window.ClickWidget(ClearButton);
    if (0 < Platinum)
    {
        EnterNumber(Window, Platinum);
        Window.ClickWidget(PlatinumButton, true);
    }
    if (0 < Gold)
    {
        EnterNumber(Window, Gold);
        Window.ClickWidget(GoldButton, true);
    }
GetPipelineManager().StartAcquiring();
//    Message;txData;
#endif
    return true;
}

////////////////////////////////////////////////////////////////////////////////

void
Handler_t::
EnterNumber(
    const Ui::Window_t& Window,
          size_t        Number)  const
{
    using namespace SetPrice::Widget::Id;
    static const Ui::WidgetId_t DigitWindowIds[] =
    {
        ZeroButton,
        OneButton,
        TwoButton,
        ThreeButton,
        FourButton,
        FiveButton,
        SixButton,
        SevenButton,
        EightButton,
        NineButton,
    };

    std::vector<size_t> Digits;
    while (0 < Number)
    {
        size_t Digit = Number % 10;
        Digits.push_back(Digit);
        Number /= 10;
    }
    std::wstring strNumber;
    while (!Digits.empty())
    {
        size_t digit = Digits.back();
        strNumber += wchar_t(L'0' + digit);
        Window.ClickWidget(DigitWindowIds[digit], true);
        Digits.pop_back();
    }
    LogAlways(L"TxRepriceItems::EnterNumber() Number(%d) String(%s)",
              Number, strNumber.c_str());
}

////////////////////////////////////////////////////////////////////////////////

bool
Handler_t::
ValidatePrice(
    const SetPrice::Translate::Data_t& Message,
          Data_t&                      txData) const
{
    if (Message.Price == txData.price)
    {
        return true;
    }
    else
    {
        LogError(L"TxRepriceItems::ValidatePrice(): Actual (%d) Expected (%d)",
                 Message.Price, txData.price);
        txData.price = 0;
        return false;
    }
}

////////////////////////////////////////////////////////////////////////////////

void
Handler_t::
ListItem(
    const Sell::Translate::Data_t& Message,
          Data_t&                  txData,
    const Ui::Window_t&            Window) const
{
    size_t Row;
    if (GetSellItemRow(Message, txData, Row))
    {
        const Sell::Text_t& text = Message.Text;
        const wchar_t* pListedText =
            &text.GetRow(Row)[text.GetColumnOffset(Sell::Table::ListedColumn)];
        const bool bListed = (0 == wcscmp(L"Yes", pListedText));
        if (!bListed)
        {
            if (0 != wcscmp(L"No", pListedText))
            {
                LogError(L"TxRepriceItems::ListItem(): Invalid listed text(%ls) row(%d)",
                         pListedText, Row);
                return;
            }
        }
        if (((0 == txData.price) && bListed) || ((0 < txData.price) && !bListed))
        {
            Window.ClickWidget(Sell::Widget::Id::ListItemButton);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

bool
Handler_t::
NextItem(
    const Sell::Translate::Data_t& /*Message*/,
          Data_t&                  txData) const
{
    size_t erased = txData.sellItemNames.erase(txData.sellItemName);
    LogAlways(L"TxRepriceItems::NextItem() erased %d x (%s) remaining (%d)",
              erased, txData.sellItemName.c_str(), txData.sellItemNames.size());
    if (txData.sellItemNames.empty())
    {
        txData.Complete();
        return false;
    }
    txData.sellItemName.clear();
    return true;
}

////////////////////////////////////////////////////////////////////////////////

void
Handler_t::
DumpItems(
    const wchar_t*       pTitle,
    const ItemDataMap_t& items) const
{
    LogAlways(L"Dump(%s)", pTitle);
    using namespace Transaction;
    ItemDataMap_t::const_iterator it = items.begin();
    for (; items.end() != it; ++it)
    {
        size_t Count = 0;
        size_t Min = 0;
        size_t Max = 0;
        using namespace BuyTabGetItems;
        PriceCountMap_t::const_iterator itMap = it->second.begin();
        for (; it->second.end() != itMap; ++itMap)
        {
            Count += itMap->second;
            const size_t Value = itMap->first;
            if ((0 == Min) || Value < Min)
            {
                Min = Value;
            }
            if (Value > Max)
            {
                Max = Value;
            }
        }
        LogAlways(L"  (%-40ls) Count(%d) Min(%d) Max(%d)",
                  it->first.c_str(), Count, Min, Max);
    }
}

////////////////////////////////////////////////////////////////////////////////

} // RepriceItems
} // Transaction
} // Broker

////////////////////////////////////////////////////////////////////////////////

