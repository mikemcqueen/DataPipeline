///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// LonTrader.cpp
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LonTrader_t.h"
#include "LonTraderImpl_t.h"
#include "PipelineManager.h"
#include "Log_t.h"
#include "Macros.h"
#include "LonCard_t.h"
#include "LonPlayer_t.h"
#include "EventTypes.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#endif

///////////////////////////////////////////////////////////////////////////////

extern bool       g_bWriteBmps;
extern DWORD      g_dwSleep;

///////////////////////////////////////////////////////////////////////////////
//
// LonTrader_t static definitions.
//

// move to a DB class of some sort
const wchar_t LonTrader_t::s_szBaseDbName[]    = L"\\db\\cards.mdb";
const wchar_t LonTrader_t::szLonPostedTrades[] = L"LonPostedTrades";

wchar_t       LonTrader_t::s_szDbName[MAX_PATH];

///////////////////////////////////////////////////////////////////////////////
//
// Constructor.
//

LonTrader_t::
LonTrader_t(
          LonWindow_t& Window,
    const wchar_t*     pszUsername)
:
    m_Window(Window),
    m_pImpl(new LonTraderImpl_t(pszUsername))
{
}

///////////////////////////////////////////////////////////////////////////////

LonTrader_t::
~LonTrader_t()
{
}

///////////////////////////////////////////////////////////////////////////////

bool
LonTrader_t::
Initialize()
{
    // TODO: objets responsible for adding themselves?
    DP::PipelineManager_t& pm = GetPipelineManager();
    using namespace DP::Stage;

	pm.AddHandler(Acquire, m_pImpl->m_SsTrades, szLonPostedTrades);
	pm.AddHandler(Acquire,   m_pImpl->m_PcapTrades,                    szLonPostedTrades);
	
	pm.AddHandler(Translate, m_pImpl->m_TrPrompts, szLonPostedTrades);
	pm.AddHandler(Translate, m_pImpl->m_TrScroll, szLonPostedTrades);
	pm.AddHandler(Translate, m_pImpl->m_PostedTrades.GetTranslator(), szLonPostedTrades);
	pm.AddHandler(Translate, m_pImpl->m_TradeDetail.GetTranslator(), szLonPostedTrades);
	pm.AddHandler(Translate, m_pImpl->m_TradeBuilder.GetTranslator(), szLonPostedTrades);
	pm.AddHandler(Translate, m_pImpl->m_ConfirmTrade.GetTranslator(), szLonPostedTrades);
	
	pm.AddHandler(Interpret, m_pImpl->m_PostedTrades.GetInterpreter(), szLonPostedTrades);
	pm.AddHandler(Interpret, m_pImpl->m_TradeDetail.GetInterpreter(), szLonPostedTrades);
	pm.AddHandler(Interpret, m_pImpl->m_TradeBuilder.GetInterpreter(), szLonPostedTrades);
	pm.AddHandler(Interpret, m_pImpl->m_ConfirmTrade.GetInterpreter(), szLonPostedTrades);

    // NOTE: Manager before Player
    // NOTE: Player before Poster
	pm.AddHandler(Analyze, m_pImpl->m_TradeManager, szLonPostedTrades);
	pm.AddHandler(Analyze, m_pImpl->m_Player, szLonPostedTrades);
	pm.AddHandler(Analyze, m_pImpl->m_TradePoster, szLonPostedTrades);
	pm.AddHandler(Analyze, m_pImpl->m_TradeExecutor, szLonPostedTrades);

    return true;
}

///////////////////////////////////////////////////////////////////////////////

PostedTrades::Manager_t&
LonTrader_t::
GetPostedTrades()
{
    return m_pImpl->m_PostedTrades;
}

///////////////////////////////////////////////////////////////////////////////


LonCardSet_t&
LonTrader_t::
GetCardSet()
{
    return LonTraderImpl_t::s_CardSet;
}

///////////////////////////////////////////////////////////////////////////////

TradeManager_t&
LonTrader_t::
GetTradeManager()
{
    return LonTraderImpl_t::m_TradeManager;
}

///////////////////////////////////////////////////////////////////////////////

TradeExecutor_t&
LonTrader_t::
GetTradeExecutor()
{
    return LonTraderImpl_t::m_TradeExecutor;
}

///////////////////////////////////////////////////////////////////////////////

TradePoster::Manager_t&
LonTrader_t::
GetTradePoster()
{
    return m_pImpl->m_TradePoster;
}

///////////////////////////////////////////////////////////////////////////////

LonPlayer_t&
LonTrader_t::
GetPlayer()
{
    return LonTraderImpl_t::m_Player;
}

///////////////////////////////////////////////////////////////////////////////

bool 
LonTrader_t::
CopyDb(
    const wchar_t* ToPath,
    const wchar_t* FromPath)
{
    if (CopyFile(FromPath, ToPath, FALSE))
        return true;
    LogError(L"CopyDb(%ls, %s): CopyFile() error, %d", 
             FromPath, ToPath, GetLastError());
    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool
LonTrader_t::
CopyDb(
         SYSTEMTIME& t,
         wchar_t*    pszNewFile,
         size_t      cNewFile,
   const wchar_t*    pszPath)
{
    std::wstring strPath(pszPath);
    size_t pos = strPath.find_last_of(L'\\');
    if (strPath.npos == pos)
        pos = 0;
    else
        ++pos;
    std::wstring strNew(strPath, 0, pos);
    strNew.append(GetTimeString(t));
    strNew.append(L"_");
    strNew.append(strPath, pos, strPath.length() - pos);
    wcscpy_s(pszNewFile, cNewFile, strNew.c_str());
    return CopyDb(pszNewFile, pszPath);
}

///////////////////////////////////////////////////////////////////////////////

void
LonTrader_t::
SetDbName(
    const wchar_t* DbName)
{
    wcscpy_s(s_szDbName, /*_countof(s_szDbName),*/ DbName);
    LogAlways(L"SetDbName(%s)", s_szDbName);
}

///////////////////////////////////////////////////////////////////////////////

const wchar_t* 
LonTrader_t::
GetDbConnectString()
{
    static const wchar_t szAltFormat[] =
        L"DSN=MS Access Database;DBQ=%ls;DriverId=25;FIL=MS Access;MaxBufferSize=2048;PageTimeout=5;UID=admin;";
    static wchar_t szConnect[256];
    wsprintf(szConnect, szAltFormat, s_szDbName);
    return szConnect;
}

///////////////////////////////////////////////////////////////////////////////

bool
LonTrader_t::
Start(
    const wchar_t* pszDbName,
          bool     bGetYourCards)
{
    LogInfo(L"LonTrader_t::Start()");

    if (0 < g_dwSleep)
    {
        LogAlways(L"Sleeping %d ms...", g_dwSleep);
        Sleep(g_dwSleep);
    }

    if (!GetCardSet().ReadCards(pszDbName))
        return false;

    size_t TradeCount = GetTradeManager().ReadAllTrades();
    if ((0 < TradeCount) || !bGetYourCards)
        GetPlayer().ReadYourCards();
    if (0 < TradeCount)
        return true;

    // Only start up the acquire handlers if we didn't read trades from DB.
    LogInfo(L"Start(): No trades in DB. Starting acquire handlers...");

    // 1 == SsTask, 2 = PcapTask.
    Lon::Event::Start_t EventStart;
    size_t Started = GetPipelineManager().SendEvent(EventStart);
    if (2 != Started)
    {
        LogError(L"Only %d acquire handler(s) started", Started);
        return false;
    }
    // there is a potential deadlock here. DP is grabbing M1 (handler list)
    // then potentially M2 (transaction manager queue)
    // functions below are grabbing (M2) TM queue, (M1) handler list

    if (bGetYourCards)
        GetPlayer().DoGetYourCards();
    GetTradeManager().DoGatherTrades();
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void
LonTrader_t::
Stop()
{
    LogInfo(L"LonTrader_t::Stop()");
    Lon::Event::Stop_t EventStop;
    GetPipelineManager().SendEvent(EventStop);
}

///////////////////////////////////////////////////////////////////////////////

void
LonTrader_t::
ReadConsoleLoop()
{
    wchar_t buf[256];
    while (ReadConsoleCommand(buf, _countof(buf)))
    {
        try
        {
            if (L'.' == buf[0])
                return;
            DispatchCommand(buf);
        }
        catch (std::exception& e)
        {
            LogError(L"DispatchCmd() Exception: %hs", e.what());
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

bool
LonTrader_t::
ReadConsoleCommand(TCHAR buf[], DWORD dwSize)
{
    HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
    DWORD dwRead;
    bool bValid = FALSE != ReadConsole(h, buf, dwSize, &dwRead, NULL);
    if (bValid)
    {
        buf[dwRead] = L'\0';
        for (--dwRead; iswspace(buf[dwRead]); --dwRead)
            buf[dwRead] = L'\0';
    }
    return bValid;
}

///////////////////////////////////////////////////////////////////////////////

void
LonTrader_t::
DispatchCommand(
    const wchar_t* buf)
{
    LogAlways(L"CMD: %ls", buf);
    bool bHandled = true;
    bool bCmd = false;
    switch(buf[0])
    {
    case L'b': bCmd = CmdBuilder(&buf[1]); break;
    case L'c': bCmd = CmdCards(&buf[1]);   break;
    case L'k': bCmd = CmdControl(&buf[1]); break;
    case L'l':
        {
            int iLevel = _wtoi(&buf[1]);
            Log_t::Get().SetLogLevel(iLevel);
            LogAlways(L"LogLevel=%d", iLevel);
            bCmd = true;
        }
        break;
    case L't': bCmd = CmdTrades(&buf[1]);  break;
//  case L'?': ShowHelp(commands);         break;
    default:   bHandled = false;           break;
    }
    if (!bHandled)
        LogError(L"Unknown command (%s)", buf);
    else if (!bCmd)
        LogError(L"Command failed (%s)", buf);
}

///////////////////////////////////////////////////////////////////////////////

bool
LonTrader_t::
CmdControl(
    const wchar_t* pszCmd)
{
    size_t Pos = 0;
    switch (pszCmd[Pos++])
    {
    case L'b':  // toggle g_bWriteBmps
        g_bWriteBmps = !g_bWriteBmps;
        LogAlways(L"WriteBmps=%d", g_bWriteBmps);
        return true;
    case L'k':  // toggle clicking
        {
            bool bClick = m_pImpl->m_SsTrades.ToggleClick();
            LogAlways(L"Click=%d", bClick);
        }
        return true;
    case L't':  // transaction manager
        if (L'c' == pszCmd[Pos])      // complete: complete the current transaction
        {
            LogError(L"Not implemented");
            return true;
            // pData = TM.Acquire()
            //GetTransactionManager().CompleteTransaction(pData->Id, Lon::ErrorTimeout);
        }
        else if (L'r' == pszCmd[Pos]) // release; this may not be so useful anymore
        {
            GetTransactionManager().Release(false);
            return true;
        }
        break;
    default:
        break;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////

bool
LonTrader_t::
CmdBuilder(
    const wchar_t* pszCmd)
{
    switch (pszCmd[0])
    {
    case L't':     // Text
        switch (pszCmd[1])
        {
        case L's': // Set
            LonWindow_t::SendChars(Lon::Window::TradeBuilderSearchEdit, &pszCmd[2]);
            return true;
        default:
            break;
        }
        break;
    default:
        break;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////
