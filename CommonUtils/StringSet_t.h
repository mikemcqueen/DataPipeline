////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
////////////////////////////////////////////////////////////////////////////////

#pragma once
  
#include "Log.h"

class StringSet_t :
    public set<wstring>
{

public:

    StringSet_t() {}

    void
    Dump() const
    {
        LogAlways(L"StringSet_t::Dump(%d)", size());
        for (const_iterator it = begin(); end() != it; ++it)
        {
            LogAlways(L"  (%s)", it->c_str());
        }
    }
};


typedef vector<wstring> StringVector_t;

