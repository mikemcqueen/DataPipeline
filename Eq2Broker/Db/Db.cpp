////////////////////////////////////////////////////////////////////////////////
//
// Db.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Db.h"
#include "Log.h"

namespace Db
{

// move to a DB class of some sort
static const wchar_t  s_szBrokerDbFormat[]      = L"\\db\\broker_%s.mdb";
static const wchar_t  s_szBrokerDbBase[]        = L"\\db\\broker.mdb";
static wchar_t        s_szBrokerDbPath[MAX_PATH];
static wchar_t        s_szBrokerDbConnect[MAX_PATH];

static bool g_bInitialized = false;
static bool g_bBrokerInitialized = false;

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
    static CDatabase s_DbBroker;
    switch (DbType)
    {
    case Broker:   return s_DbBroker;
    default:
        throw std::invalid_argument("Db::GetDb()");
    }
}

///////////////////////////////////////////////////////////////////////////////

bool
OpenAll()
{
    DWORD Flags = CDatabase::noOdbcDialog;
    if (!g_bInitialized)
    {
        throw std::logic_error("DbOpenAll(): Not initialized");
    }
    if (g_bBrokerInitialized)
    {
        try
        {
            if (!GetDb(Broker).OpenEx(Db::GetConnectString(Broker), Flags))
            {
                LogError(L"Db::OpenAll(): Open Broker failed");
                return false;
            }
            LogInfo(L"Global Broker DB opened");
        }
        catch (CDBException* e)
        {
            LogError(L"ItemForSale_t::OpenAll() exception: %ls", (LPCTSTR)e->m_strError);
            e->Delete();
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void
CloseAll()
{
    if (g_bBrokerInitialized)
    {
        GetDb(Broker).Close();
    }
}

///////////////////////////////////////////////////////////////////////////////

bool
Initialize(
    const wchar_t* pBrokerDbPath,
    const wchar_t* pServerName,
          bool     bInitBroker,
          DWORD    /*ThreadId*/)
{
    if ((NULL == pServerName) || (L'\0' == pServerName[0]))
    {
        throw std::invalid_argument("Db::Initialize()");
    }
    if (g_bInitialized)
    {
        throw std::runtime_error("Db::Intialize() called twice");
    }
    if (bInitBroker)
    {
        if ((NULL == pBrokerDbPath) || (L'\0' == pBrokerDbPath[0]))
        {
            swprintf_s(s_szBrokerDbPath, s_szBrokerDbFormat, pServerName);
            SYSTEMTIME LocalTime;
            GetLocalTime(&LocalTime);
            wchar_t DbName[MAX_PATH];
            if (!Db::CopyDb(LocalTime, DbName, _countof(DbName)))
            {
                return false;
            }
            SetDbName(DbName);
        }
        else
        {
            SetDbName(pBrokerDbPath);
        }
        g_bBrokerInitialized = true;
        s_szBrokerDbConnect[0] = '\0';
    }
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
Cleanup(
    DWORD /*ThreadId*/)
{
    CloseAll();
}

///////////////////////////////////////////////////////////////////////////////

bool 
CopyDb(
    const wchar_t* ToPath,
    const wchar_t* FromPath)
{
    if (NULL == FromPath)
    {
        FromPath = s_szBrokerDbBase;
    }
    if (!CopyFile(FromPath, ToPath, FALSE))
    {
        LogError(L"CopyDb(%ls, %s): CopyFile() error, %d", 
                 FromPath, ToPath, GetLastError());
        return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool
CopyDb(
    SYSTEMTIME&    t,
    wchar_t*       pszNewFile,
    size_t         cNewFile,
    const wchar_t* FromPath)
{
    if (NULL == FromPath)
    {
        FromPath = s_szBrokerDbPath;
    }
    std::wstring strPath(FromPath);
    size_t pos = strPath.find_last_of(L'\\');
    if (strPath.npos == pos)
    {
        pos = 0;
    }
    else
    {
        ++pos;
    }
    std::wstring strNew(strPath, 0, pos);
    strNew.append(GetTimeString(t));
    strNew.append(L"_");
    strNew.append(strPath, pos, strPath.length() - pos);
    wcscpy_s(pszNewFile, cNewFile, strNew.c_str());
    return CopyDb(pszNewFile, FromPath);
}

///////////////////////////////////////////////////////////////////////////////

void
SetDbName(
    const wchar_t* DbName)
{
    wcscpy_s(s_szBrokerDbPath, /*_countof(s_szDbName),*/ DbName);
    LogAlways(L"SetDbName(%s)", s_szBrokerDbPath);
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
    wchar_t* pConnect = NULL;
    const wchar_t* pDbPath = NULL;

    switch (DbType)
    {
    case Broker:
        if (!g_bBrokerInitialized)
        {
            throw std::logic_error("Db::GetConnectString(): !bBrokerInitialized");
        }
        pConnect = s_szBrokerDbConnect;
        pDbPath  = s_szBrokerDbPath;
        break;
    default:
        throw std::invalid_argument("Db::GetConnectString() invalid DbType");
    }
    ASSERT(NULL != pConnect);
    if (L'\0' == pConnect[0])
    {
        ASSERT((NULL != pDbPath) && (L'\0' != pDbPath[0]));
        FormatConnectString(pConnect, MAX_PATH, pDbPath);
    }
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
        L"DSN=MS Access Database;DBQ=%ls;DriverId=25;FIL=MS Access;"
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

} // Db

////////////////////////////////////////////////////////////////////////////////
