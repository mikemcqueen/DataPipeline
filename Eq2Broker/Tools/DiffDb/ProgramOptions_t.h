////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//

#pragma once

#include "Flag_t.h"
#include "boost/program_options.hpp"

namespace DiffDb
{

namespace Flag
{
    enum Type : unsigned
    {
        None                        = 0,
        NoRemoveAllAfterMismatch    = 0x0001,
        RevalueRaisedPrices         = 0x0002,
        IgnoreRemovedSellers        = 0x0004,
        RemoveHighSales             = 0x0008,
        DumpHiLo                    = 0x0010,
        DumpBuySell                 = 0x0020,

        NoFirstDb                   = 0x0100,
    };

    extern const char* g_szInputFile;//                = "input-file";
    extern const char* g_szNoRemoveAllAfterMismatch;// = "noremoveallaftermismatch";
    extern const char* g_szRevalueRaisedPrices;//      = "revalueraisedprices";
    extern const char* g_szIgnoreRemovedSellers;//     = "ignoreremovedsellers";
    extern const char* g_szRemoveHighSales;//          = "removehighsales";
    extern const char* g_szDumpHiLo;//                 = "dumphilo";
    extern const char* g_szDumpBuySell;//              = "dumpbuysell";
}

//typedef FlagBase_t<Flag::Type> Flag_t;

namespace po = boost::program_options;

class ProgramOptions_t
{

private:

    static Flag_t s_Flags;

public:

    static po::variables_map s_variableMap;

    ProgramOptions_t(){}

    const po::variables_map&
    Parse(int argc, wchar_t* argv[]);

    bool
    ValidateProgramOptions(
        const po::variables_map&       vm,
        const po::options_description& desc);

    Flag_t
    GetFlags(
        const po::variables_map& vm) const;

    void SetFlags(Flag_t Flags) { s_Flags = Flags; }
    static Flag_t GetFlags() { return s_Flags; }
};

namespace PO
{
    template<class Ty>
    const Ty& GetOption(const char* option)
    {
        return ProgramOptions_t::s_variableMap[option].as<Ty>();
    }

    Flag_t
    GetFlags();
}

} // namespace DiffDb
