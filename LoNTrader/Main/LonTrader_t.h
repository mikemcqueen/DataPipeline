/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// LonTrader_t.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_LONTRADER_T_H
#define Include_LONTRADER_T_H

#include "resource.h"
#include "TradePosterTypes.h"

/////////////////////////////////////////////////////////////////////////////

struct LonTraderImpl_t;
class LonCardSet_t;
class LonWindow_t;
class LonPlayer_t;
class TradeManager_t;
class TradeExecutor_t;

namespace TradePoster  { class Manager_t; }
namespace PostedTrades { class Manager_t; }

class LonTrader_t final
{

public:

    static const wchar_t    szLonPostedTrades[];
    static const wchar_t    s_szBaseDbName[];

    static wchar_t          s_szDbName[MAX_PATH];

 
    static LonCardSet_t& GetCardSet();
    static TradeManager_t&  GetTradeManager();
    static TradeExecutor_t& GetTradeExecutor();
    static LonPlayer_t&     GetPlayer();

private:

    std::unique_ptr<LonTraderImpl_t> m_pImpl;
    LonWindow_t&     m_Window;

public:

    // Constructor & Destructor:

    LonTrader_t(
        LonWindow_t& Window,
        const wchar_t* pUsername);

    LonTrader_t(const LonTrader_t&) = delete;
    LonTrader_t(LonTrader_t&&) = delete;

    ~LonTrader_t(); /* = default */

    bool
    Initialize();

    bool
    Start(
        const wchar_t* pszDbName,
        bool bGetYourCards);

    void
    Stop();

    void
    ReadConsoleLoop();

    static
    void
    SetDbName(
        const wchar_t* DbName);

    static
    const wchar_t* 
    GetDbConnectString();

    static
    bool 
    CopyDb(
        const wchar_t* ToPath,
        const wchar_t* FromPath = s_szBaseDbName);

    static
    bool
    CopyDb(
              SYSTEMTIME& Time,
              wchar_t*    pszNewFile,
              size_t      cNewFile,
        const wchar_t*    pszPath  = s_szBaseDbName);

private:

    // Accessors:

    PostedTrades::Manager_t& GetPostedTrades();
    TradePoster::Manager_t&  GetTradePoster();

    bool
    ReadConsoleCommand(
        wchar_t* buf,
        DWORD    dwSize);

    void
    DispatchCommand(
        const wchar_t* buf);

    size_t
    ReadTradeXmls(
        const wchar_t* pszWildcard);

    size_t
    WriteTradeXmls(
        const wchar_t* pszDir);

    bool
    CmdCards(
        const wchar_t* pszCmd);

    bool
    CmdTrades(
        const wchar_t* pszCmd);

    bool
    CmdTradePosterGetArgs(
        const wchar_t*     pszCmd,
        TradePoster::Id_t& Id,
        size_t&            Value,
        std::wstring*      pstrOtherArgs = NULL);

    bool
    CmdTradePoster(
        const wchar_t* pszCmd,
              bool     bTest);

    bool
    CmdTradePosterCard(
        const wchar_t* pszCmd);

    bool
    CmdTradePosterTrade(
        const wchar_t* pszCmd,
              bool     bTest);

    bool
    CmdTradePosterPrice(
        const wchar_t* pszCmd);
//              bool     bTest);

    bool
    CmdTradePosterValue(
        const wchar_t* pszCmd,
        bool bTest);

    bool
    CmdTradePosterBuySellCard(
             bool     bBuy,
       const wchar_t* pszCmd,
             bool     bTest);

    bool
    CmdControl(
        const wchar_t* pzzCmd);

    bool
    CmdBuilder(
        const wchar_t* pzzCmd);
   
private:

    LonTrader_t() = delete;
    LonTrader_t& operator=(const LonTrader_t&) = delete;
};

/////////////////////////////////////////////////////////////////////////////

#endif // Include_LONTRADER_T_H

/////////////////////////////////////////////////////////////////////////////
