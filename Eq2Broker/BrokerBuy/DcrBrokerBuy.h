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
#include "DcrWindowPolicy.h"
#include "DcrTable_t.h"
#include "DcrRect_t.h"
#include "BrokerBuyText.h"
#include "Macros.h"
#include "Rect.h"
#include "BrokerId.h"
#include "BrokerUi.h"
#include "PageNumber_t.h"

namespace Broker::Buy::Translate {
  constexpr auto kSearchTextMax = 101;
  constexpr auto kSavedSearchMax = 30;

  namespace Legacy {
    struct Data_t : public DP::Message::Data_t {
      Buy::Text_t tableText;
      int selectedRow;
      PageNumber_t pageNumber;
      char searchText[kSearchTextMax];
      bool searchBoxHasCaret;
      char savedSearch[kSavedSearchMax];

      Data_t(
        const TextTable_t& textTable,
        size_t initSelectedRow,
        const PageNumber_t& initPageNumber,
        const string& initSearchText,
        bool initSearchBoxHasCaret,
        const string& initSavedSearch) :
        DP::Message::Data_t(
          DP::Stage_t::Translate,
          Message::Id::Buy,
          sizeof(Data_t),
          kMsgName),
        tableText(textTable.GetData()),
        selectedRow(initSelectedRow),
        pageNumber(initPageNumber),
        searchBoxHasCaret(initSearchBoxHasCaret)
      {
        strcpy_s(searchText, initSearchText.c_str());
        strcpy_s(savedSearch, initSavedSearch.c_str());
      }
    };
  } // namespace Legacy

  typedef SsWindow::Acquire::Data_t             AcquireData_t;
  typedef DcrWindow::Policy::Translate::Many_t  TranslatePolicy_t;
  typedef DcrWindow::Policy::NoValidate_t       ValidatePolicy_t;

  using BaseHandler_t = DcrWindow::Translate::Handler_t<
    TranslatePolicy_t, ValidatePolicy_t>;

  class Handler_t : public BaseHandler_t {
    friend struct Translate::Data_t;

  public:
    explicit Handler_t(const Window_t& window);

    // Handler_t virtual:
    bool PreTranslateSurface(
      CSurface* pSurface,
      Ui::WindowId_t windowId,
      int dcrId,
      Rect_t* pRect) const override;

    void PostData(DWORD /*Unused*/) const override;

    const Text_t& GetText() const { return m_TextTable.GetData(); }
    const DcrTable_t& GetDcr() const { return m_DcrTable; }

  public:
    static void SaveImageWithWidgetRects(
      std::wstring_view name,
      const CSurface* pSurface,
      const Rect_t& rc,
      std::span<const Ui::Widget::Data_t> widgets);

  private:
    const Window_t& window_;
    TranslatePolicy_t m_TranslatePolicy;
    ValidatePolicy_t  m_ValidatePolicy;
    DcrVector_t       m_DcrVector; // TODO get rid of this, pass by value
    DcrTable_t        m_DcrTable;
    TextTable_t       m_TextTable;
    DcrRect_t         m_DcrSearchEdit;
    DcrRect_t         m_DcrSearchDropdown;
    DcrRect_t         m_DcrPageNumber;
  };
} // Broker::Buy::Translate

#endif // Include_DCRBROKERBUY_H
