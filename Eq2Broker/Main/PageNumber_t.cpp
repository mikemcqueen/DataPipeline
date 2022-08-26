/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// PageNumber_t.cpp
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PageNumber_t.h"

/////////////////////////////////////////////////////////////////////////////

bool
PageNumber_t::
Parse(
    const wchar_t* pText)
{
    static const wchar_t szPage[]       = L"Page ";
    static const size_t  PageLength     = _countof(szPage) - 1;
    static const wchar_t szLastPage[]   = L" of ";
    static const size_t  LastPageLength = _countof(szLastPage) - 1;

    if (0 != wcsncmp(pText, szPage, PageLength))
        return false;
    pText += PageLength;
    size_t Page = _wtoi(pText);
    if (0 == Page)
        return false;
    while (L'\0' != *pText && iswdigit(*pText))
        ++pText;
    if (0 != wcsncmp(pText, szLastPage, LastPageLength))
        return false;
    pText += LastPageLength;
    size_t LastPage = _wtoi(pText);
    if (0 == LastPage)
        return false;

    m_Data.Page = Page;
    m_Data.LastPage = LastPage;
    return true;
}

/////////////////////////////////////////////////////////////////////////////
