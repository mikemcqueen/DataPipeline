////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ProgramOptions_t.h"
#include "BuyItems.h"
#include "Log.h"
#include "Timer_t.h"

////////////////////////////////////////////////////////////////////////////////

namespace BuyItems
{

///////////////////////////////////////////////////////////////////////////////

namespace Flag
{
    const char* g_szInputFile                = "input-file";
}

///////////////////////////////////////////////////////////////////////////////

po::variables_map ProgramOptions_t::s_variableMap;
Flag_t ProgramOptions_t::s_Flags;

///////////////////////////////////////////////////////////////////////////////

bool
ProgramOptions_t::
ValidateProgramOptions(
    const po::variables_map&       vm,
    const po::options_description& desc)
{
    static const int MaxThreadCount = 8;

    using namespace Flag;
    if (vm.count("usage"))
    {
        cout << endl << desc << endl;
        return false;
    }
    int logLevel = vm["loglevel"].as<int>();
    Log::SetLevel(logLevel);
    LogAlways(L"LogLevel:  %d", logLevel);
    LogAlways(L"Server:    %ls", vm["server"].as<wstring>().c_str());
/*
    if (0 < vm.count(g_szInputFile))
    {
        const vector<wstring>& InputFiles = vm[g_szInputFile].as<vector<wstring> >();
        if ((2 != InputFiles.size()) || !vm["directory"].defaulted())
        {
            cout << "Either --directory or 2 x --" << g_szInputFile << " must be specified";
            return false;
        }
        LogAlways(L"Files:     %ls %ls", InputFiles[0].c_str(), InputFiles[1].c_str());
    }
    else
    {
        LogAlways(L"Directory: %ls", vm["directory"].as<wstring>().c_str());
    }
    if (vm["dumpcsv"].as<bool>())
    {
        LogAlways(L"Dumping CSV");
    }
    int threadCount = vm["threads"].as<int>();
    if (threadCount < 1 || threadCount > MaxThreadCount)
    {
        LogError(L"Invalid thread count (%d)", threadCount);
        return false;
    }
    if (vm[g_szDumpHiLo].as<bool>() && vm[g_szDumpBuySell].as<bool>())
    {
        LogError(L"Can't dump both hilo and buysell data");
        return false;
    }
*/
    return true;
}

////////////////////////////////////////////////////////////////////////////////

const po::variables_map&
ProgramOptions_t::
Parse(
    int      argc,
    wchar_t* argv[])
{
    const int      DefaultLogLevel      = 1;

    const wchar_t* DefaultServerNameW = L"mistmoore";
    const char*    DefaultServerNameA =  "mistmoore";
    const wchar_t* DefaultDirectoryW  = L"\\db";
    const char*    DefaultDirectoryA  =  "\\db";

    locale::global(locale(""));

    using namespace Flag;

    static po::positional_options_description positionalOptions;
    positionalOptions.add(g_szInputFile, 2);

    static po::options_description commandLineOptions("Allowed options:");
    commandLineOptions.add_options()
        ("usage,?",                                                                "show usage")
        ("loglevel,l",     po::value<int>()->default_value(DefaultLogLevel),       "set log level (0-3)")
        ("server,s",       po::wvalue<wstring>()->default_value(DefaultServerNameW, DefaultServerNameA), "set server name")
        ("directory,d",    po::wvalue<wstring>()->default_value(DefaultDirectoryW, DefaultDirectoryA),   "set db directory")
        ;

    po::variables_map& variableMap = s_variableMap;
    po::store(po::basic_command_line_parser<wchar_t>(argc, argv).
              options(commandLineOptions).positional(positionalOptions).run(),
              variableMap);
    po::notify(variableMap);

    if (!ValidateProgramOptions(variableMap, commandLineOptions))
    {
        exit(0);
    }
    return variableMap;
}

////////////////////////////////////////////////////////////////////////////////

Flag_t
ProgramOptions_t::
GetFlags(
    const po::variables_map& vm) const
{
    using namespace Flag;
    struct
    {
        const char*        pszOption;
        BuyItems::Flag::Type Flag;
    } OptionFlag[] = 
    {
        "dummy", (BuyItems::Flag::Type)0
    };
    Flag_t Flags;
    for (int Index = 0; Index < _countof(OptionFlag); ++Index)
    {
        if (vm[OptionFlag[Index].pszOption].as<bool>())
        {
            LogAlways(L"Flag:   %hs", OptionFlag[Index].pszOption);
            Flags.Set(OptionFlag[Index].Flag);
        }
    }
    return Flags;
}

namespace PO
{
    Flag_t
    GetFlags()
    {
        return ProgramOptions_t::GetFlags();
    }
}

////////////////////////////////////////////////////////////////////////////////

} // namespace BuyItems

////////////////////////////////////////////////////////////////////////////////
