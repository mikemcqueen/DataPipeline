////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//

#pragma once

#include "Flag_t.h"

namespace BuyItems
{
    namespace Flag
    {
        enum Type : unsigned
        {
            None                        = 0,
        };

        extern const char* g_szInputFile;//                = "input-file";
    }

//typedef FlagBase_t<Flag::Type> Flag_t;


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

} // namespace BuyItems
