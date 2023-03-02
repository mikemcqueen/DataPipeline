////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// MainWindow_t.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainWindow_t.h"
#include "OtherWindows.h"
#include "Log.h"

MainWindow_t::MainWindow_t() :
  Ui::Window::WithHandle_t(
    Ui::Window::Id::MainWindow,
    "EQ2ApplicationClass",
    "MainWindow",
    { broker_frame_ }),
  broker_frame_(*this)
{}
