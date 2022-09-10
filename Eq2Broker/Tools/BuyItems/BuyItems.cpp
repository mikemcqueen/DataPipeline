///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// BuyItems.cpp
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BuyItems.h"
#include "Log_t.h"
#include "Log.h"
#include "Timer_t.h"
#include "ProgramOptions_t.h"
#include "AccountsDb.h"
#include "DbItems_t.h"

// The one and only application object

CWinApp theApp;

namespace BuyItems
{

///////////////////////////////////////////////////////////////////////////////

bool
StartupInitialize(
    const wchar_t* pServerName)
{
	if (!AfxWinInit(::GetModuleHandle(nullptr), nullptr, ::GetCommandLine(), 0))
	{
		wprintf(L"MFC initialization failed\n");
	    return false;
	}
    if (!Log_t::Get().Initialize())
    {
        wprintf(L"Log_t::Initialize() failed\n");
        return false;
    }
    if (!Log_t::Get().Open(L"BuyItems"))
    {
        wprintf(L"Log_t::Open() failed, %d\n", GetLastError());
        return false;
    }
/*
    if (!Db::Initialize(nullptr, pServerName, false))
    {
        wprintf(L"Db::Initialize(%ls) failed", pServerName);
        return false;
    }
*/
    if (!Accounts::Db::Initialize(pServerName))
    {
        wprintf(L"AccountsDb::Initialize(%s) failed", pServerName);
        return false;
    }
    return true;
}

extern const char* g_szInputFile;

////////////////////////////////////////////////////////////////////////////////

} // namespace BuyItems

////////////////////////////////////////////////////////////////////////////////

struct Cleanup_t
{
    Cleanup_t() {}
    ~Cleanup_t()
    {
        Accounts::Db::Cleanup();
        Log_t::Get().Shutdown();
    }
};

////////////////////////////////////////////////////////////////////////////////

bool
Execute(
    const po::variables_map& vm)
{
    // open buy\server.csv
    // read each line, split it
    // calculate high bid, low ask, item id,
    // dump items_to_buy_sell format line for .csv
    wstring path(L"buy\\");
    path.append(vm["server"].as<wstring>().c_str());
    path.append(L".csv");
    wifstream ifs(path.c_str());
    int line = 0;
    for (line = 1; ifs.good(); ++line)
    {
        wchar_t buf[256];
        ifs.getline(buf, _countof(buf));
        wstring str(buf);
        vector<wstring> args;
        boost::split(args, str, boost::is_any_of(L","));
        if (2 == args.size())
        {
            ItemId_t itemId = Accounts::Db::Items_t::GetItemId(args[0].c_str());
            if (0 != itemId)
            {
                double margin = .2;
                double fee = .2;
                size_t target = _wtoi(args[1].c_str());
                size_t highBid = (size_t)((1.0 - fee) * target);
                size_t lowAsk  = (size_t)((1.0 + margin) * target);
                size_t highAsk  = 2 * lowAsk;
                size_t maxToOwn = 2;
                wprintf(L"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                    line, itemId,
                    0, highBid, 0,
                    lowAsk, highAsk, 0,
                    0, 0,
                    maxToOwn, 0, 0);
            }
            else
            {
                LogError(L"Unknown item (%s)", args[0].c_str());
                return false;
            }
        }
        else
        {
            LogError(L"Skipped Line(%d) args(%d)", line, args.size());
            return false;
        }
    }
    LogAlways(L"Read %d lines", line);
    return true;
}

////////////////////////////////////////////////////////////////////////////////

int
wmain(
    int argc,
    wchar_t* argv[])
{
    Cleanup_t Cleanup;
    try
    {
        using namespace BuyItems;
        ProgramOptions_t programOptions;
        const po::variables_map& vm = programOptions.Parse(argc, argv);
//        programOptions.SetFlags(programOptions.GetFlags(vm));
        if (!StartupInitialize(vm["server"].as<wstring>().c_str()))
        {
            return -1;
        }
        Execute(vm);
    }
    catch (exception& e)
    {
        LogError(L"### Caught %hs: %hs ###", typeid(e).name(), e.what());
    }
    catch (CDBException* e)
    {
        LogError(L"### Caught CDBException: %s ###", (LPCTSTR)e->m_strError);
        e->Delete();
    }
    catch (...)
    {
        LogError(L"### Unhandled exception ###");
    }
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
