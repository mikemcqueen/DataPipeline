////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// DcrSetPrice.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SetPrice.h"
#include "DcrBase_t.h"        // hack , to get charset, should live elsewhere
#include "PipelineManager.h"
#include "SsWindow.h"
#include "DdUtil.h"
#include "BrokerId.h"
#include "Price_t.h"

namespace Broker
{
namespace SetPrice
{
namespace Translate
{

//DP::Message::Id_t Data_t::s_MessageId      = Broker::Message::Id::Buy; // DP::Message::Id::Unknown;

////////////////////////////////////////////////////////////////////////////////

Handler_t::
Handler_t(
    Window::ManagerBase_t& windowManager)
:
    HandlerBase_t(
        TopWindowId,
        m_TranslatePolicy,
        m_ValidatePolicy,
        L"SetPrice"),
    m_TranslatePolicy(m_DcrVector),
    m_DcrPrice(
        windowManager.GetWindow(),
        Widget::Id::PriceText,
        DcrBase_t::GetCharset()),
    m_Manager(windowManager)
{
    m_DcrVector.push_back(&m_DcrPrice);
}

////////////////////////////////////////////////////////////////////////////////

void
Handler_t::
PostData(
    DWORD /*Unused*/)
{
    bool ExtraLog = 0;

    Price_t Price;
    if (!Price.Parse(m_DcrPrice.GetText().c_str()))
    {
        LogError(L"DcrSetPrice::PostData(): Price.Parse(%ls) failed",
                 m_DcrPrice.GetText().c_str());
        return;
    }
    if (ExtraLog)
    {
        LogAlways(L"DcrSetPrice::PostData() Price(%d)", Price);
    }
    void *pBuffer = GetPipelineManager().Alloc(sizeof(Data_t));
    if (nullptr == pBuffer)
    {
        LogError(L"DcrSetPrice::PostData(): Alloc callback data failed.");
    }
    else
    {
        LogInfo(L"DcrSetPrice::PostData() Text (%s)", m_DcrPrice.GetText().c_str());
        Data_t* pData = new (pBuffer)
            Data_t(
                GetClass().c_str(),
                Price.GetPrice());
        HRESULT hr = GetPipelineManager().Callback(pData);
        if (FAILED(hr))
        {
            LogError(L"DcrSetPrice::PostData(): PM.Callback() failed.");
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

} // Translate
} // SetPrice
} // Broker

////////////////////////////////////////////////////////////////////////////////
