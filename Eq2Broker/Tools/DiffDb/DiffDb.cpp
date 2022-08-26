///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// DiffDb.cpp : Defines the entry point for the console application.
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DiffDb.h"
#include "DiffDbFaster_t.h"
#include "Log_t.h"
#include "Log.h"
#include "Db.h"
#include "Timer_t.h"
#include "DbItems_t.h"
#include "DiffDbTypes.h"
#include "DiffDbUtil.h"
#include "ProgramOptions_t.h"
#include "DiffDir_t.h"
#include "AccountsDb.h"

// The one and only application object

CWinApp theApp;

namespace DiffDb 
{

///////////////////////////////////////////////////////////////////////////////

bool
StartupInitialize(
    const wchar_t* pServerName)
{
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		wprintf(L"MFC initialization failed\n");
	    return false;
	}
    if (!Log_t::Get().Initialize())
    {
        wprintf(L"Log_t::Initialize() failed\n");
        return false;
    }
    if (!Log_t::Get().Open(L"DiffDb"))
    {
        wprintf(L"Log_t::Open() failed, %d\n", GetLastError());
        return false;
    }
    if (!Db::Initialize(NULL, pServerName, false))
    {
        wprintf(L"Db::Initialize(%ls) failed", pServerName);
        return false;
    }
    if (!Accounts::Db::Initialize(pServerName))
    {
        wprintf(L"AccountsDb::Initialize(%s) failed", pServerName);
        return false;
    }
    return true;
}

extern const char* g_szInputFile;

////////////////////////////////////////////////////////////////////////////////

} // namespace DiffDb;

////////////////////////////////////////////////////////////////////////////////

struct Cleanup_t
{
    Cleanup_t() {}
    ~Cleanup_t()
    {
        Db::Cleanup();
        Log_t::Get().Shutdown();
    }
};

////////////////////////////////////////////////////////////////////////////////

int
wmain(
    int argc,
    wchar_t* argv[])
{
    Cleanup_t Cleanup;
    try
    {
        using namespace DiffDb;
        ProgramOptions_t programOptions;
        const po::variables_map& vm = programOptions.Parse(argc, argv);
        programOptions.SetFlags(programOptions.GetFlags(vm));
        if (!StartupInitialize(vm["server"].as<wstring>().c_str()))
        {
            return -1;
        }
        DbFiles_t dbFiles;
        if (0 < vm.count(Flag::g_szInputFile))
        {
            const vector<wstring>& InputFiles = vm[Flag::g_szInputFile].as<vector<wstring> >();
            dbFiles.SetFiles(InputFiles[0], InputFiles[1]);
        }
        else
        {
            dbFiles.SetDirectory(vm["directory"].as<wstring>());
        }
        DiffDir_t diffDir;
        diffDir.Diff(dbFiles, vm["threads"].as<int>());
    }
    catch (exception& e)
    {
        LogError(L"### Caught %hs: %hs ###", typeid(e).name(), e.what());
    }
    catch (...)
    {
        LogError(L"### Unhandled exception ###");
    }
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
#if 0

namespace std
{
    // ADL fixes.
    wostream& operator<<(wostream& s, const wstring& ws) 
    {
        wstring::const_iterator it = ws.begin();
        for (; ws.end() != it; ++it)
        {
            s << (wchar_t(*it));
        }
        return s;
    }
    template<class T>
    wostream& operator<<(wostream& s, const vector<T>& v) 
    {
        vector<T>::const_iterator it = v.begin();
        for (; v.end() != it; ++it)
       {
            s << *it << " ";
        }
        return s;
    }
}
#endif

////////////////////////////////////////////////////////////////////////////////
