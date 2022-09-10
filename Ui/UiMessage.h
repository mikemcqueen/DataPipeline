/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// UiMessage.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_UIMESSAGE_H
#define Include_UIMESSAGE_H

#include "UiWindowId.h"
#include "DpMessage.h"

namespace Ui
{
namespace Message
{

/*
    namespace Id
    {
        enum : unsigned
        {
            Click,
            SendChars,
            Scroll,
            ThumbPosition,
            Collection,
        };
    }
*/
/*
    struct Data_t :
        DP::Message::Data_t
    {
        WindowId_t WindowId;

        Data_t(
            DP::Stage_t            Stage        = DP::Stage::Any,
            DP::MessageId_t Id           = DP::Message::Id::Unknown,
            Ui::WindowId_t      InitWindowId = Window::Id::Unknown,
            size_t            Size         = sizeof(Data_t),
            const wchar_t*    pClass       = nullptr)
        :
            DP::Message::Data_t(
                Stage,
                Id,
                Size,
                pClass),
            WindowId(InitWindowId)
        { }        
    };
*/
} // Message
} // Ui

/////////////////////////////////////////////////////////////////////////////

#endif // Include_UIMESSAGE_H

/////////////////////////////////////////////////////////////////////////////
