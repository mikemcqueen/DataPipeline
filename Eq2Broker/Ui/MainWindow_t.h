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

class MainWindow_t : public Ui::Window::WithHandle_t {
public:
  MainWindow_t();

  auto& GetBrokerWindow() const { return broker_frame_; }

private:
  Broker::Window_t broker_frame_;
};

#endif // INCLUDE_MAINWINDOW_T_H