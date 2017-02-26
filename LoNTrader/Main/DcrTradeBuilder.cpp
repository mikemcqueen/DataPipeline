/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// DcrTradeBuilder.cpp
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DcrTradeBuilder.h"
#include "SsTrades_t.h"
#include "PipelineManager.h"
#include "LonWindow_t.h"

namespace TradeBuilder
{
namespace Translate
{

/////////////////////////////////////////////////////////////////////////////

Handler_t::
Handler_t() :
    TrWindow_t(TopWindowType, m_TranslatePolicy, m_ValidatePolicy),
    m_TranslatePolicy(TopWindowType, m_Dcr, this, &m_rcBounds),
    m_Dcr(&m_Text,
          Table::TheirPixelColumnWidths,
          _countof(Table::TheirPixelColumnWidths),
          Table::LineHeightPixels),
    m_Text(
        Table::TheirCharColumnWidths,
        _countof(Table::TheirCharColumnWidths)),
    m_DcrWindowType(DcrTheirWindowType)
{
    SetRect(&m_rcBounds, 0, 0, 0, 0);
    m_Dcr.SetGridline(1);
}

/////////////////////////////////////////////////////////////////////////////

Lon::Window::Type_e
Handler_t::
GetDcrWindowType() const
{
    return m_DcrWindowType;
}

/////////////////////////////////////////////////////////////////////////////

void
Handler_t::
PostData(
    DWORD /*AcquireId*/)
{
    Translate::Data_t* pData = static_cast<Translate::Data_t*>
        (GetPipelineManager().Alloc(sizeof(Translate::Data_t)));
    if (NULL == pData)
    {
        LogError(L"DcrPostedTradeBuilder_t::PostData(): Alloc callback data failed.");
    }
    else
    {
        m_Text.Dump(L"DcrPostedTradeBuider_t::PostData()");
        pData->Initialize(this);
        HRESULT hr = GetPipelineManager().Callback(pData);
        ASSERT(SUCCEEDED(hr));
    }
}

/////////////////////////////////////////////////////////////////////////////

void
Data_t::
Initialize(
    const Handler_t* pClass)
{
    wcscpy_s(Class, pClass->GetClass().c_str());
    Stage      = DP::Stage::Translate;
    Type       = DP::Message::Type::Message;
    Id         = Lon::Message::Id::TextTable;
    WindowType = TopWindowType;
    Text       = pClass->m_Text.GetData();
    Rect       = pClass->m_rcBounds;
}

/////////////////////////////////////////////////////////////////////////////
//
// Hack.
// TODO: we can "translate" and "interpret" the your/mine buttons to 
//       determine what state we're in, yes?
//

void
Handler_t::
SetCollection(
    const Collection_e Coll)
{
    CLock lock(m_csState);
    switch (Coll)
    {
    case Collection::Theirs:
        m_Dcr.SetColumnWidths(Table::TheirPixelColumnWidths, Table::TheirColumnCount);
        m_Text.SetColumnWidths(Table::TheirCharColumnWidths, Table::TheirColumnCount);
        m_DcrWindowType = DcrTheirWindowType;
        break;
    case Collection::Yours:
        m_Dcr.SetColumnWidths(Table::YourPixelColumnWidths, Table::YourColumnCount);
        m_Text.SetColumnWidths(Table::YourCharColumnWidths, Table::YourColumnCount);
        m_DcrWindowType = DcrYourWindowType;
        break;
    default:
        ASSERT(false);
        break;
    }
}

/////////////////////////////////////////////////////////////////////////////

} // Translate
} // TradeBuilder

/////////////////////////////////////////////////////////////////////////////
