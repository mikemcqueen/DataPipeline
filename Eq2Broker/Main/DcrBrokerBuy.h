/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// DcrBrokerBuy.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_DCRBROKERBUY_H
#define Include_DCRBROKERBUY_H

#include "DcrWindow.h"
#include "DcrBase_t.h"
#include "BrokerBuyText.h"
#include "Macros.h"
#include "Rect.h"
#include "DcrRect_t.h"
#include "BrokerId.h"
#include "BrokerUi.h"
#include "PageNumber_t.h"

namespace Broker
{
namespace Buy
{
namespace Translate
{

////////////////////////////////////////////////////////////////////////////////

constexpr auto kSearchTextMax = 101;
constexpr auto kSavedSearchMax = 30;

struct Data_t :
    public DP::Message::Data_t
{
    Buy::Text_t  tableText;
    size_t       selectedRow;
    PageNumber_t pageNumber;
    wchar_t      searchText[kSearchTextMax];
    bool         searchBoxHasCaret;
    wchar_t      savedSearch[kSavedSearchMax];

    Data_t(
        const wchar_t*       pClass,
        const TextTable_t&   textTable,
        size_t               initSelectedRow,
        const PageNumber_t&  initPageNumber,
        const wstring&       initSearchText,
        bool                 initSearchBoxHasCaret,
        const wstring&       initSavedSearch)
    :
        DP::Message::Data_t(
            DP::Stage::Translate,
            Message::Id::Buy,
            sizeof(Data_t),
            pClass),
        tableText(textTable.GetData()),
        selectedRow(initSelectedRow),
        pageNumber(initPageNumber),
        searchBoxHasCaret(initSearchBoxHasCaret)
    {
        wcscpy_s(searchText, initSearchText.c_str());
        wcscpy_s(savedSearch, initSavedSearch.c_str());
    }

private:

    Data_t();
};

////////////////////////////////////////////////////////////////////////////////

typedef SsWindow::Acquire::Data_t             AcquireData_t;
typedef DcrWindow::Policy::TranslateMany_t    TranslatePolicy_t;
typedef DcrWindow::Policy::NoValidate_t       ValidatePolicy_t;

typedef DcrWindow::Translate::Handler_t<
            TranslatePolicy_t,
            ValidatePolicy_t>                 HandlerBase_t;

class Handler_t :
    public HandlerBase_t
{
    friend struct Translate::Data_t; 

    Window::ManagerBase_t& m_windowManager;

    TranslatePolicy_t m_TranslatePolicy;
    ValidatePolicy_t  m_ValidatePolicy;
    DcrVector_t       m_DcrVector;
    DcrBase_t         m_DcrTable;
    TextTable_t       m_TextTable;
    DcrRect_t         m_DcrSearchEdit;
    DcrRect_t         m_DcrSearchDropdown;
    DcrRect_t         m_DcrPageNumber;

public:

    Handler_t(
        Window::ManagerBase_t& windowManager);

    //
    // DcrWindow virtual:
    //
        
    void
    PostData(
        DWORD /*Unused*/) override;

public:

    const Text_t&     GetText() const  { return m_TextTable.GetData(); }

    const DcrBase_t&  GetDcr() const   { return m_DcrTable; }
    DcrBase_t&        GetDcr()         { return m_DcrTable; }

private:

    Handler_t();
    Handler_t(const Handler_t&);
    Handler_t& operator=(const Handler_t&);
};

/////////////////////////////////////////////////////////////////////////////

} // Translate
} // Buy
} // Broker

#endif // Include_DCRBROKERBUY_H

/////////////////////////////////////////////////////////////////////////////
