/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// Dcr.cpp
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Dcr.h"
#include "Charset_t.h"
#include "DdUtil.h"
#include "Log.h"
#include "TextTable_t.h"
#include "Macros.h"
#include "Rect.h"

bool g_bWriteBmps = true;
bool g_bTableFixColor = true;

//struct DCR::ImplInterface_t;
/*static*/
std::unordered_map<DcrImpl, std::unique_ptr<DCR::ImplInterface_t>> DCR::impl_map_{};

DCR::DCR(
    int id,
    std::optional<DcrImpl> method)
    :
    id_(id),
    method_(method.value_or(DcrImpl::Default))
{ }


DCR::~DCR() = default;

void
DCR::
WriteBadBmp(
    const CSurface* pSurface,
    const RECT& rc,
    const wchar_t* pszText)
{
    WCHAR szFile2[MAX_PATH];
    wsprintf(szFile2, L"Diag\\dcr_bad_%s.bmp", pszText);
    for (WCHAR* p = szFile2; L'\0' != *p; ++p) {
        if (nullptr != wcschr(L",'?", *p)) {
            *p = L'_';
        }
    }
    pSurface->WriteBMP(szFile2, rc);
}
