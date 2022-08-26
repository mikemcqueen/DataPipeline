////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// Db.h
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Db
{
    enum Type_e
    {
        Broker,        // a.k.a. ForSale, items_for_sale table, this is the daily market data dump db, server specific
    };

    bool
    Initialize(
        const wchar_t* pBrokerDbName,        // user supplied override
        const wchar_t* pServerName,
              bool     bInitBroker,
              DWORD    ThreadId = 0);

    void
    Cleanup(
        DWORD ThreadId = 0);

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

    bool 
    CopyDb(
        const wchar_t* ToPath,
        const wchar_t* FromPath = NULL);

    bool
    CopyDb(
              SYSTEMTIME& t,
              wchar_t*    pszNewFile,
              size_t      cNewFile,
        const wchar_t*    FromPath = NULL);

    void
    SetDbName(
        const wchar_t* DbName);

    void
    Escape(
        const wchar_t*      pItemName,
              std::wstring& strItem);

    void
    SetTimestamp(
        TIMESTAMP_STRUCT& ts,
        const CTime&      t);

}