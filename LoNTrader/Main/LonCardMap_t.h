/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// LonCardSet_t.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_LONCARDMAP_T_H
#define Include_LONCARDMAP_T_H

#include "LonCard_t.h"

/////////////////////////////////////////////////////////////////////////////

class IdCardMap_t :
    public std::map<CardId_t, const LonCard_t*, LonCard_t::CompareId>
{
public:

    typedef std::pair<CardId_t, const LonCard_t*> Pair_t;
};

/////////////////////////////////////////////////////////////////////////////

class LonCardMap_t :
    public std::map<const std::wstring, LonCard_t, LonCard_t::CompareName>
{
public:

    typedef std::pair<const std::wstring, LonCard_t> Pair_t;

private:

    std::wstring m_strName;

public:

    LonCardMap_t(
        const wchar_t* pszName)
    :
        m_strName(pszName)
    { }

    const wchar_t*
    GetName() const
    {
        return m_strName.c_str();
    }

private:

    LonCardMap_t();
    LonCardMap_t(const LonCardMap_t&);
    LonCardMap_t& operator=(const LonCardMap_t&);
};

/////////////////////////////////////////////////////////////////////////////

#endif // Include_LONCARDSET_T_H

/////////////////////////////////////////////////////////////////////////////
