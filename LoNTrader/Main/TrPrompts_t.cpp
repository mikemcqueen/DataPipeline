/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TrPrompts_t.cpp
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TrPrompts_t.h"

using namespace Lon;

/////////////////////////////////////////////////////////////////////////////

bool
TrPrompts_t::
IsSupportedWindowType(
    Window::Type_e Type) const
{
    return Window::Unknown != GetDismissWindowType(Type);
}

/////////////////////////////////////////////////////////////////////////////

Window::Type_e
TrPrompts_t::
GetDismissWindowType(
    Window::Type_e Type) const
{
    using namespace Window;
    static const
    struct
    {
        Type_e ParentType;
        Type_e DismissType;
    } MapParentToDismiss[] =
    {
        { SystemMessageWindow,    SystemMessageClose        },
        { NetworkStatusWindow,    NetworkStatusOk           },
        { TradeBuilderExitPrompt, TradeBuilderExitPromptYes },
        { AcceptTradeWindow,      AcceptTradeYes            },
        { DeliveryWindow,         DeliveryClose             },
        { JoinGuildWindow,        JoinGuildNo               },
        { CancelTradeWindow,      CancelTradeYes            },
    };
    for (size_t Entry = 0; _countof(MapParentToDismiss) > Entry; ++Entry)
    {
        if (MapParentToDismiss[Entry].ParentType == Type)
            return MapParentToDismiss[Entry].DismissType;
    }
    return Unknown;
}

/////////////////////////////////////////////////////////////////////////////


HRESULT
TrPrompts_t::
MessageHandler(
    const DP::Message::Data_t* pMessage)
{
    if (!Validate(pMessage))
        return S_FALSE;

    Window::Type_e
        WindowType = static_cast<const Message::Data_t*>(pMessage)->WindowType;
    // TODO: Save Screenshot.
    LogWarning(L"Dismissing prompt window (%s)",
               LonWindow_t::GetWindowTitle(WindowType));
    LonWindow_t::Click(GetDismissWindowType(WindowType));
    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
