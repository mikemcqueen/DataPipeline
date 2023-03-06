/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// DcrBrokerSell.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_DCRBROKERSELL_H
#define Include_DCRBROKERSELL_H

#include "DcrWindow.h"
#include "DcrWindowPolicy.h"
#include "DcrTable_t.h"
#include "DcrRect_t.h"
#include "BrokerSellText.h"
#include "BrokerSellTypes.h"
#include "Macros.h"
#include "Rect.h"
#include "BrokerId.h"
#include "BrokerUi.h"
#include "dp_msg.h"

namespace Broker::Sell::Translate {
  struct Data_t : dp::msg::Data_t {
    Data_t(int i) : dp::msg::Data_t(kMsgName) { i; } // TEMP

    Data_t(const Table::RowVector rws, int sel_row,
      Ui::Scroll::Position_t vs_pos) :
      dp::msg::Data_t(kMsgName),
      rows(std::move(rws)),
        selected_row(sel_row),
        vscroll_pos(vs_pos)
      {}

    Table::RowVector rows;
    int selected_row;
    Ui::Scroll::Position_t vscroll_pos;
  };

  namespace msg {
    inline auto validate(const dp::Msg_t& msg) {
      return dp::msg::validate<Data_t>(msg, kMsgName);
    }
  }

  namespace Legacy {
    struct Data_t : DP::Message::Data_t {
      typedef Sell::Text_t Text_t;
      Text_t                 Text;
      size_t                 selectedRow;
      Ui::Scroll::Position_t VScrollPos;

      Data_t(
        const TextTable_t& TextTable,
        size_t           initSelectedRow,
        Ui::Scroll::Position_t InitVScrollPos) :
        DP::Message::Data_t(
          DP::Stage_t::Translate,
          Message::Id::Sell,
          sizeof(Data_t),
          kMsgName),
        Text(TextTable.GetData()),
        selectedRow(initSelectedRow),
        VScrollPos(InitVScrollPos)
      { }

    private:
      Data_t();
    };
  } // namespace Legacy

  typedef DcrWindow::Policy::Translate::Many_t TranslatePolicy_t;
  typedef DcrWindow::Policy::NoValidate_t      ValidatePolicy_t;

  using BaseHandler_t = DcrWindow::Translate::Handler_t<
    TranslatePolicy_t, ValidatePolicy_t>;

  class Handler_t : public BaseHandler_t {
    friend struct Translate::Data_t;

  public:
    explicit Handler_t(const Window_t& window); // Window::ManagerBase_t& Manager);

    // Handler_t virtual:
    bool PreTranslateSurface(
      CSurface* pSurface,
      Ui::WindowId_t windowId,
      int dcrId,
      Rect_t* pRect) const override;
  
    void PostData(DWORD /*Unused*/) const override;

    const Text_t& GetText() const { return m_TextTable.GetData(); }
    const DcrTable_t& GetDcr() const { return m_DcrTable; }

  private:
//    Window::ManagerBase_t& GetManager() const { return m_windowManager; }

  private:
//    Window::ManagerBase_t& m_windowManager;
    const Window_t& window_;
    TranslatePolicy_t m_TranslatePolicy;
    ValidatePolicy_t m_ValidatePolicy;
    DcrVector_t m_DcrVector;
    TextTable_t m_TextTable;
    DcrTable_t  m_DcrTable;
    DcrRect_t m_DcrSetPrice;
    DcrRect_t m_DcrListItem;
  };
} // Broker::Sell::Translate

#endif // INCLUDE_DCRBROKERSELL_H
