////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// MainWindow_t.h
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef INCLUDE_MAINWINDOW_T_H
#define INCLUDE_MAINWINDOW_T_H

#include "UiWindow.h"
#include "BrokerWindow.h"
#include "BrokerUi.h"
#include "DpMessage.h"

class MainWindow_t : public Ui::Window::WithHandle_t {
public:
  MainWindow_t();

  // Ui::Window_t virtual:
//  Ui::Window::Base_t& GetWindow(Ui::WindowId_t WindowId) const override;
  Ui::WindowId_t GetWindowId(const CSurface& Surface,
    const POINT* pptHint) const override;

  auto& GetBrokerWindow() const { return broker_frame_; }

  Ui::WindowId_t GetMessageWindowId(const DP::MessageId_t& messageId) const;

private:
  Broker::Window_t broker_frame_;
};

#endif // INCLUDE_MAINWINDOW_T_H