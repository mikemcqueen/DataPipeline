/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// TxSetWidgetText.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_TXSETWIDGETTEXT_H
#define Include_TXSETWIDGETTEXT_H

#include "DpHandler_t.h"
#include "UiWindowId.h"
#include "BrokerId.h"

class Eq2Broker_t;

namespace Broker
{
namespace Transaction
{
namespace SetWidgetText
{

/////////////////////////////////////////////////////////////////////////////

namespace State
{
    enum : DP::Transaction::State_t
    {
        First                    = DP::Transaction::State::User_First,
        ClearText                = First,
        EnterText,
        ValidateText
    };
}

/////////////////////////////////////////////////////////////////////////////

struct Data_t :
    public DP::Transaction::Data_t
{
    Ui::WindowId_t windowId;
    Ui::WidgetId_t widgetId;
    Ui::WidgetId_t altFocusWidgetId;
    wstring        text;

    Data_t(
        Ui::WindowId_t initWindowId,
        Ui::WidgetId_t initWidgetId,
        const wstring& initText,
        Ui::WidgetId_t initAltFocusWidgetId = Ui::Widget::Id::Unknown)
        //TODO: clearTextWidgetId = buttontoclicktocleartext
    :
        DP::Transaction::Data_t(
            Id::SetWidgetText,
            sizeof(Data_t)),
        windowId(initWindowId),
        widgetId(initWidgetId),
        text(initText),
        altFocusWidgetId(initAltFocusWidgetId)
    {
    }

private:

    Data_t();
};

/////////////////////////////////////////////////////////////////////////////

class Handler_t final :
    public DP::Handler_t
{
private:

    Eq2Broker_t& m_broker;

public:

    Handler_t(Eq2Broker_t& broker) :
        m_broker(broker)
    {}

    //
    // DP::Handler_t virtual
    //
 
    HRESULT
    MessageHandler(
        const DP::Message::Data_t* pData) override;

    HRESULT
    ExecuteTransaction(
        DP::Transaction::Data_t& Data) override;

    HRESULT
    OnTransactionComplete(
        const DP::Transaction::Data_t& Data) override;

private:

    void
    ClearText(
        Data_t& txData,
        const wstring& currentText) const;
};

} // SetWidgetText
} // Transaction
} // Broker

/////////////////////////////////////////////////////////////////////////////

#endif // Include_TXSetWidgetText_H

/////////////////////////////////////////////////////////////////////////////
