////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//


#include "stdafx.h"
#include "DiffDbUtil.h"

////////////////////////////////////////////////////////////////////////////////

namespace DiffDb
{

const wchar_t*
GetCoinString(
    size_t   Value,
    wchar_t* pBuffer,
    size_t   BufferCount)
{
    static wchar_t Buffer[32];
    if (NULL == pBuffer)
    {
        pBuffer = Buffer;
        BufferCount = _countof(Buffer);
    }
    else if (0 == BufferCount)
    {
        throw std::invalid_argument("GetCoinString()");
    }
    pBuffer[0] = L'\0';
    size_t Plat = Value / 100;
    size_t Gold = Value % 100;
    if (0 < Plat)
    {
        swprintf_s(pBuffer, BufferCount, L"%dp", Plat);
        if (0 < Gold)
        {
            wcscat_s(pBuffer, BufferCount, L", ");
        }
    }
    if (0 < Gold)
    {
        wchar_t szGold[8];
        swprintf_s(szGold, L"%dg", Gold);
        if (0 < Plat)
        {
            wcscat_s(pBuffer, BufferCount, szGold);
        }
        else
        {
            wcscpy_s(pBuffer, BufferCount, szGold);
        }
    }
    if (0 == Value)
    {
        wcscpy_s(pBuffer, BufferCount, L"0");
    }
    return pBuffer;
}

} // namespace DiffDb

////////////////////////////////////////////////////////////////////////////////
