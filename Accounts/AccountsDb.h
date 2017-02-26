////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// AccountsDb.h
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Accounts
{
namespace Db
{

    enum Type_e
    {
        Items,         // all items ever discovered, global to all servers
        Sellers,       // sellers, server specific
        BuySell,       // items to buy/sell, server specific
    };

    bool
    Initialize(
        const wchar_t* pServerName);

    void
    Cleanup();

    const wchar_t*
    GetConnectString(
        Type_e DbType);

    const wchar_t* 
    FormatConnectString(
              wchar_t* pBuffer,
              size_t   Count,
        const wchar_t* pDbPath);

    CDatabase&
    GetDb(
        Type_e DbType);

    void
    Escape(
        const wchar_t*      pItemName,
              std::wstring& strItem);

    void
    SetTimestamp(
        TIMESTAMP_STRUCT& ts,
        const CTime&      t);

} // Db
} // Accounts
