/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// Price_t.cpp
//
// Price_t class.
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Price_t.h"
#include "Log.h"

bool Price_t::Parse(
  std::string_view text,
  bool bAllowSmallCoin /*= true*/)
{
  LogInfo(L"Price_t::Parse(%S)", text.data());
  const char* pCoin = text.data();
  bool bSmallCoin = false;
  size_t Price = 0;
  for (;;) {
    while ((L'\0' != *pCoin) && !isdigit(*pCoin)) {
      ++pCoin;
    }
    if (L'\0' == *pCoin) {
      break;
    }
    long Coin = atol(pCoin);
#if 0 // 0c is valid coin string
    if (0 == Coin) {
      LogError(L"Price_t::Parse(): Coin = 0 @ (%ls) FullText (%ls)", pCoin, pText);
      return false;
    }
#endif
    while (isdigit(*pCoin)) {
      ++pCoin;
    }
    switch (*pCoin) {
    case 'p':
      Coin *= 100;
      break;
    case 'g':
      break;
    case 's':
    case 'c':
      if (0 < Coin) {
        if (bAllowSmallCoin) {
          Coin = 0;
          bSmallCoin = true;
          break;
        }
        LogWarning(L"Price_t::Parse(): Small coin not allowed (%d%C)", Coin, *pCoin);
        return false;
      }
      break;
    default:
      LogError(L"Price_t::Parse(): Unknown coin type (%C)", *pCoin);
      return false;
    }
    Price += Coin;
  }
  if (bSmallCoin) {
    ++Price;
  }
  m_Data.Price = Price;
  return true;
}

/*
////////////////////////////////////////////////////////////////////////////////
// TODO: Misc.cpp
const wchar_t*
GetCoinString(
    size_t   Value,
    wchar_t* pBuffer,
    size_t   BufferCount)
{
    static wchar_t Buffer[32];
    if (nullptr == pBuffer)
    {
        pBuffer = Buffer;
        BufferCount = _countof(Buffer);
    } else if (0 == BufferCount)
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
            wcscat_s(pBuffer, BufferCount, L",");
        }
    }
    if (0 < Gold)
    {
        wchar_t szGold[8];
        swprintf_s(szGold, L"%dg", Gold);
        if (0 < Plat)
        {
            wcscat_s(pBuffer, BufferCount, szGold);
        } else
        {
            wcscpy_s(pBuffer, BufferCount, szGold);
        }
    }
    if (0 == Value)
    {
        wcscpy_s(pBuffer, BufferCount, L"0c");
    }
    return pBuffer;
}
*/
/////////////////////////////////////////////////////////////////////////////
