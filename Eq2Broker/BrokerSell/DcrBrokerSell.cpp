////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// DcrBrokerSell.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BrokerSellWindowManager.h"
#include "PipelineManager.h"
#include "SsWindow.h"
#include "DdUtil.h"
#include "BrokerId.h"
#include "BrokerUi.h"

namespace Broker::Sell::Translate {
  constexpr Rect_t TextRects[Table::ColumnCount] = {
    { 2, 28, // first gap + quantitytexttop 
      2 + Table::PixelColumnWidths[0] + 2, 28 + 12 }, // first gap + width + fudge, quantitytexttop + quantitytextheight

    { 2 + Table::PixelColumnWidths[0], 0,  // first gap + first column pixel width
      2 + Table::PixelColumnWidths[0] + Table::PixelColumnWidths[1],
      Broker::Table::RowHeight },

    { 2 + Table::PixelColumnWidths[0] + Table::PixelColumnWidths[1], 0, // first gap + two column pixel widths
      2 + Table::PixelColumnWidths[0] + Table::PixelColumnWidths[1] + Table::PixelColumnWidths[2],
      Broker::Table::RowHeight },

    { 2 + Table::PixelColumnWidths[0] + Table::PixelColumnWidths[1] + Table::PixelColumnWidths[2], 0,
      2 + Table::PixelColumnWidths[0] + Table::PixelColumnWidths[1] + Table::PixelColumnWidths[2] + Table::PixelColumnWidths[3],
      Broker::Table::RowHeight },
  };

  constexpr TableParams_t TableParams = {
      Broker::Table::RowHeight,//+ Broker::Table::GapSizeY,
      Broker::Table::CharHeight,
      Broker::Table::RowGapSize,
      Table::ColumnCount
  };

  Handler_t::Handler_t(Window::ManagerBase_t& Manager) :
    BaseHandler_t(
      kWindowId,
      m_TranslatePolicy,
      m_ValidatePolicy,
      L"BrokerSell"),
    m_TranslatePolicy(
      *this,
      m_DcrVector),
    m_DcrTable(
      Widget::Id::Table,
      {},
      &m_TextTable,
      TableParams,
      std::span{ Table::PixelColumnWidths },
      std::span{ TextRects }),
    m_TextTable(
      std::span{ Table::CharColumnWidths }),
    m_DcrSetPrice(
      Widget::Id::SetPriceButton),
    m_DcrListItem(
      Widget::Id::ListItemButton),
    m_windowManager(Manager)
  {
    m_DcrVector.push_back(&m_DcrTable);
    m_DcrVector.push_back(&m_DcrSetPrice);
    m_DcrVector.push_back(&m_DcrListItem);
  }

  bool Handler_t::PreTranslateSurface(
    CSurface* pSurface,
    Ui::WindowId_t windowId,
    int dcrId,
    Rect_t* pRect) const
  {
    windowId;
    pSurface;

    switch (dcrId) {
    case Widget::Id::Table:
    {
      //rcSurface;
      Rect_t rect = m_windowManager.GetWindow().GetClientRect();
      if (!rect.IsEmpty()) {
        rect.top += Broker::Table::TopRowOffset;
        *pRect = rect;
        return true;
      }
    }
    break;

    default:
      m_windowManager.GetWindow().GetWidgetRect(dcrId, pRect);
      return true;
    }
    return false;
  }

  void Handler_t::PostData(DWORD /*Unused*/) const {
    extern bool g_noDcrPost;
    if (g_noDcrPost) {
      return;
    }
    void* pBuffer = GetPipelineManager().Alloc(sizeof(Data_t));
    if (nullptr != pBuffer) {
      m_TextTable.GetData().Dump(L"DcrBrokerSell");
      LogInfo(L"SeletecedRow(%d)", m_DcrTable.GetSelectedRow().value_or(-1));
      Data_t* pData = new (pBuffer) Data_t(
        GetClass().c_str(),
        m_TextTable,
        m_DcrTable.GetSelectedRow().value_or(-1),
        Ui::Scroll::Position::Unknown); // TODO:
        // GetManager().GetWindow().GetScrollPosition(Ui::Scroll::Bar::Vertical));
      GetPipelineManager().Callback(pData);
    } else {
      LogError(L"DcrBrokerSell::PostData(): Alloc callback data failed.");
    }
  }
} // namespace Broker::Sell::Translate
