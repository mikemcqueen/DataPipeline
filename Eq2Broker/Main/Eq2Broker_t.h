/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// Eq2Broker_t.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_EQ2BROKER_T_H
#define Include_EQ2BROKER_T_H

//#include "BrokerUi.h"
//#include "UiTypes.h"
//#include "MainWindow_t.h"

class Eq2BrokerImpl_t;
class CSurface;
class MainWindow_t; // probably hack

namespace Game {
  struct Options_t {
    std::wstring characterName{ L"Human" };
    std::wstring serverName{ L"Mistmoore" };
    std::wstring testImagePath;
  };
}

class Eq2Broker_t {
public:
  Eq2Broker_t(
    MainWindow_t& mainWindow,
    const Game::Options_t& options);

  ~Eq2Broker_t();

  Eq2Broker_t() = delete;
  Eq2Broker_t(const Eq2Broker_t&) = delete;
  Eq2Broker_t& operator=(const Eq2Broker_t&) = delete;

  bool Initialize();
  bool Start();
  void Stop();
  void ReadConsoleLoop();

  MainWindow_t& GetMainWindow() { return mainWindow_; }
//  Ui::Window_t& GetWindow(Ui::WindowId_t windowId);

private:

  bool InitDb(const wchar_t* pDbName);

  bool InitHandlers();

  bool CommandLoop(wchar_t* buf, DWORD size);

  bool ReadConsoleCommand(
    wchar_t* buf,
    DWORD    dwSize);

  bool DispatchCommand(const wchar_t* buf);

  bool CmdControl(const wchar_t* pzzCmd);
  bool CmdTransaction(const wchar_t* pszCmd);
  bool CmdCharacter(const wchar_t* pszCmd);

  void LoadAndSendTestImage(const wstring& testImagePath);

private:
  MainWindow_t& mainWindow_;
  const Game::Options_t& m_options;
  std::unique_ptr<Eq2BrokerImpl_t> m_pImpl;
};

#endif // Include_EQ2BROKER_T_H
