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

#include "BrokerUi.h"
#include "UiTypes.h"

/////////////////////////////////////////////////////////////////////////////

class Eq2BrokerImpl_t;
class CSurface;

class Eq2Broker_t
{

public:

    static const wchar_t*   s_pClass;

private:

    Broker::MainWindow_t&     m_mainWindow;
    std::unique_ptr<Eq2BrokerImpl_t> m_pImpl;

public:

    // Constructor & destructor:
    Eq2Broker_t(
        Broker::MainWindow_t& Window);

    ~Eq2Broker_t();

    bool
    Initialize();

    bool
    Start();

    void
    Stop();

    void
    ReadConsoleLoop();

    Broker::MainWindow_t&
    GetMainWindow();

    Ui::Window_t&
    GetWindow(
        Ui::WindowId_t windowId);

    // Hacky?
    Ui::WindowId_t
    GetWindowId(
        const CSurface& surface);

private:

    bool
    InitDb(
        const wchar_t* pDbName);

    bool
    InitHandlers();

    bool
    CommandLoop(wchar_t* buf, DWORD size);

    bool
    ReadConsoleCommand(
        wchar_t* buf,
        DWORD    dwSize);

    bool
    DispatchCommand(
        const wchar_t* buf);

    bool
    CmdControl(
        const wchar_t* pzzCmd);
   
    bool
    CmdTransaction(
        const wchar_t* pszCmd);

    bool
    CmdCharacter(
        const wchar_t* pszCmd);

private:

    // Explicitly disabled:
    Eq2Broker_t();
    Eq2Broker_t(const Eq2Broker_t&);
    Eq2Broker_t& operator=(const Eq2Broker_t&);
};

const wchar_t*
GetCoinString(
    size_t   Value,
    wchar_t* pBuffer = nullptr,
    size_t   BufferCount = 0);

/////////////////////////////////////////////////////////////////////////////

#endif // Include_EQ2BROKER_T_H

/////////////////////////////////////////////////////////////////////////////
