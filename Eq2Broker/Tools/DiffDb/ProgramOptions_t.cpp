////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ProgramOptions_t.h"
#include "DiffDb.h"
#include "DiffDbFaster_t.h"
#include "Log.h"
#include "Db.h"
#include "Timer_t.h"
#include "DbItems_t.h"
#include "DiffDbTypes.h"
#include "DiffDbUtil.h"

////////////////////////////////////////////////////////////////////////////////

namespace DiffDb
{

///////////////////////////////////////////////////////////////////////////////

namespace Flag
{
    const char* g_szInputFile                = "input-file";
    const char* g_szNoRemoveAllAfterMismatch = "noremoveallaftermismatch";
    const char* g_szRevalueRaisedPrices      = "revalueraisedprices";
    const char* g_szIgnoreRemovedSellers     = "ignoreremovedsellers";
    const char* g_szRemoveHighSales          = "removehighsales";
    const char* g_szDumpHiLo                 = "dumphilo";
    const char* g_szDumpBuySell              = "dumpbuysell";
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
    const int      DefaultThreadCount   = 1;
    const int      DefaultSortOrder     = int(CountType::NetSales);
    const size_t   DefaultMinCount      = 5;
    const size_t   DefaultSaleCountMin  = 5;
    const size_t   DefaultAvgPriceMin   = 100;
    const size_t   DefaultAvgPriceMax   = 0;
    const size_t   DefaultSigma         = 2;

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
        ("sortorder,o",    po::value<int>()->default_value(DefaultSortOrder),      "sort order (0=net sales,1=gross sales,2=sales volume)")
        ("threads,t",      po::value<int>()->default_value(DefaultThreadCount),    "thread count")
        ("sigma,s",        po::value<size_t>()->default_value(DefaultSigma),       "std deviation multiplier")
        ("dumpcsv",        po::bool_switch(),                                      "dump sale data in csv format")
        ("showdots",       po::bool_switch(),                                      "show dots")
        (g_szDumpHiLo,     po::bool_switch(),                                      "dump hi/lo sale data -or-")
        (g_szDumpBuySell,  po::bool_switch(),                                      "dump buy/sell data:")
        ("countmin,c",     po::value<size_t>()->default_value(DefaultMinCount),    "  minimum 'count' required")
        ("avgsalemin,m",   po::value<size_t>()->default_value(DefaultAvgPriceMin), "  minimum average sale price required")
        ("avgsalemax,x",   po::value<size_t>()->default_value(DefaultAvgPriceMax), "  maximum average sale price required")
        ("salecountmin",   po::value<size_t>()->default_value(DefaultSaleCountMin),"  minimum sale count required")

        (g_szInputFile,                po::wvalue<vector<wstring> >(),             "input files")
        (g_szNoRemoveAllAfterMismatch, po::bool_switch(),                          "don't remove all after price mismatch")
        (g_szRevalueRaisedPrices,      po::bool_switch(),                          "revalue raised prices")
        (g_szIgnoreRemovedSellers,     po::bool_switch(),                          "ignore sales from removed sellers")
        (g_szRemoveHighSales,          po::bool_switch(),                          "high sales values don't count towards averages") 
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
        DiffDb::Flag::Type Flag;
    } OptionFlag[] = 
    {
        g_szNoRemoveAllAfterMismatch,    Flag::NoRemoveAllAfterMismatch,
        g_szRevalueRaisedPrices,         Flag::RevalueRaisedPrices,
        g_szIgnoreRemovedSellers,        Flag::IgnoreRemovedSellers,
        g_szRemoveHighSales,             Flag::RemoveHighSales,
        g_szDumpHiLo,                    Flag::DumpHiLo,
        g_szDumpBuySell,                 Flag::DumpBuySell,
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

} // namespace DiffDb

////////////////////////////////////////////////////////////////////////////////
