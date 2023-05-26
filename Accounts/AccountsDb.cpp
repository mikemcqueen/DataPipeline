////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// AccountsDb.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AccountsDb.h"
#include "Log.h"

namespace Accounts
{
namespace Db
{

// move to a DB class of some sort
static const wchar_t  s_szItemsDbPath[]         = L"\\db\\items.mdb";
static const wchar_t  s_szSellersDbFormat[]     = L"\\db\\sellers_%s.mdb";
static const wchar_t  s_szBuySellDbFormat[]     = L"\\db\\buysell_%s.mdb";

static wchar_t        s_szSellersDbPath[MAX_PATH];
static wchar_t        s_szBuySellDbPath[MAX_PATH];

static wchar_t        s_szItemsDbConnect[MAX_PATH];
static wchar_t        s_szSellersDbConnect[MAX_PATH];
static wchar_t        s_szBuySellDbConnect[MAX_PATH];

static bool g_bInitialized = false;

///////////////////////////////////////////////////////////////////////////////

const wchar_t* 
FormatConnectString(
    wchar_t*       pBuffer,
    size_t         Count,
    const wchar_t* pDbName);

///////////////////////////////////////////////////////////////////////////////

CDatabase&
GetDb(
    Type_e DbType)
{
    static CDatabase s_DbItems;
    static CDatabase s_DbSellers;
    static CDatabase s_DbBuySell;

    switch (DbType)
    {
    case Items:    return s_DbItems;
    case Sellers:  return s_DbSellers;
    case BuySell:  return s_DbBuySell;
    default:
        throw std::invalid_argument("Accounts::Db::GetDb()");
    }
}

///////////////////////////////////////////////////////////////////////////////

bool
OpenAll()
{
    LogInfo(L"Using Global DBs...");
    DWORD Flags = CDatabase::noOdbcDialog;
    if (!g_bInitialized)
    {
        throw std::logic_error("DbOpenAll(): Not initialized");
    }
    if (!GetDb(Items).OpenEx(Db::GetConnectString(Items), Flags))
    {
        LogError(L"Db::OpenAll(): Open Items failed");
        return false;
    }
    LogInfo(L"Global Items DB opened");
 /*
    if (!GetDb(Sellers).OpenEx(Db::GetConnectString(Sellers), Flags))
    {
        LogError(L"Db::OpenAll(): Open Sellers failed");
        return false;
    }
    LogInfo(L"Global Sellers DB opened");
    if (!GetDb(BuySell).OpenEx(Db::GetConnectString(BuySell), Flags))
    {
        LogError(L"Db::OpenAll(): Open BuySell failed");
        return false;
    }
    LogInfo(L"Global BuySell DB opened");
 */
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void
CloseAll()
{
    GetDb(Items).Close();
    GetDb(Sellers).Close();
    GetDb(BuySell).Close();
}

///////////////////////////////////////////////////////////////////////////////

bool
Initialize(
    const wchar_t* pServerName)
{
    if ((nullptr == pServerName) || (L'\0' == pServerName[0]))
    {
        throw std::invalid_argument("Accounts::Db::Initialize()");
    }
    if (g_bInitialized)
    {
        throw std::runtime_error("Accounts::Db::Intialize() called twice");
    }
    swprintf_s(s_szSellersDbPath, s_szSellersDbFormat, pServerName);
    s_szSellersDbConnect[0] = '\0';
    swprintf_s(s_szBuySellDbPath, s_szBuySellDbFormat, pServerName);
    s_szBuySellDbConnect[0] = '\0';
    g_bInitialized = true;
    if (!OpenAll())
    {
        CloseAll();
        return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void
Cleanup()
{
    CloseAll();
}

///////////////////////////////////////////////////////////////////////////////

const wchar_t* 
GetConnectString(
    Type_e DbType)
{
    if (!g_bInitialized)
    {
        throw std::logic_error("Db::GetConnectString(): !bInitialized");
    }
    wchar_t* pConnect = nullptr;
    const wchar_t* pDbPath = nullptr;

    switch (DbType)
    {
    case Items:
        pConnect = s_szItemsDbConnect;
        pDbPath  = s_szItemsDbPath;
        break;
    case Sellers:
        pConnect = s_szSellersDbConnect;
        pDbPath  = s_szSellersDbPath;
        break;
    case BuySell:
        pConnect = s_szBuySellDbConnect;
        pDbPath  = s_szBuySellDbPath;
        break;
    default:
        throw std::invalid_argument("Db::GetConnectString()");
    }
    ASSERT(nullptr != pConnect);
    if (L'\0' == pConnect[0])
    {
        ASSERT((nullptr != pDbPath) && (L'\0' != pDbPath[0]));
        FormatConnectString(pConnect, MAX_PATH, pDbPath);
    }
    LogInfo(L"DB connect string: %ls", pConnect);
    return pConnect;
}

///////////////////////////////////////////////////////////////////////////////

const wchar_t* 
FormatConnectString(
          wchar_t* pBuffer,
          size_t   Count,
    const wchar_t* pDbPath)
{
    static const wchar_t szConnectFormat[] =
        L"DSN=Microsoft Access Driver;DBQ=%ls;DriverId=25;FIL=MS Access;"
        L"MaxBufferSize=2048;PageTimeout=5;UID=admin;";
    swprintf(pBuffer, Count, szConnectFormat, pDbPath);
    return pBuffer;
}

////////////////////////////////////////////////////////////////////////////////

void
Escape(
    const wchar_t* pItemName,
    std::wstring&  strItem)
{
    for(strItem.clear(); L'\0' != *pItemName; ++pItemName)
    {
        if (L'\'' == *pItemName)
            strItem.append(2, *pItemName);
        else
            strItem.append(1, *pItemName);
    }
}

////////////////////////////////////////////////////////////////////////////////

void
SetTimestamp(
    TIMESTAMP_STRUCT& ts,
    const CTime&      t)
{
    ts.day     = static_cast<SQLUSMALLINT>(t.GetDay());
    ts.hour    = static_cast<SQLUSMALLINT>(t.GetHour());
    ts.minute  = static_cast<SQLUSMALLINT>(t.GetMinute());
    ts.second  = static_cast<SQLUSMALLINT>(t.GetSecond());
    ts.month   = static_cast<SQLUSMALLINT>(t.GetMonth());
    ts.year    = static_cast<SQLUSMALLINT>(t.GetYear());
    ts.fraction = 0;
}

////////////////////////////////////////////////////////////////////////////////

} // Db
} // Accounts

////////////////////////////////////////////////////////////////////////////////
