////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// Character_t.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Character_t.h"
#include "DbItemsOwned_t.h"
#include "DbItemsToBuySell_t.h"
#include "DbItems_t.h"
#include "Log.h"

////////////////////////////////////////////////////////////////////////////////

using namespace Accounts::Db;

BuySellData_t::
BuySellData_t(
    const ItemsToBuySell_t& rs)
:
    lowBid(rs.m_LowBid),
    highBid(rs.m_HighBid),
    incBid(rs.m_BidIncrement),
    lowAsk(rs.m_LowAsk),
    highAsk(rs.m_HighAsk),
    incAsk(rs.m_AskIncrement),
    maxToOwn(rs.m_MaxToOwn),
    maxToSell(rs.m_MaxToSell),
    maxForSale(rs.m_MaxForSale)
{
}

////////////////////////////////////////////////////////////////////////////////

Character_t& GetCharacter()
{
    return Character_t::GetCharacter();
}

////////////////////////////////////////////////////////////////////////////////

Character_t* s_pCharacter;

/*static*/
void
Character_t::
SetCharacter(
    Character_t* pChar)
{
    s_pCharacter = pChar;
}

////////////////////////////////////////////////////////////////////////////////

/*static*/
Character_t&
Character_t::
GetCharacter()
{
    if (nullptr == s_pCharacter)
    {
        throw logic_error("Character_t::GetCharacter()");
    }
    return *s_pCharacter;
}

////////////////////////////////////////////////////////////////////////////////

void
Character_t::
Reload()
{
    LogWarning(L"Character_t::Reload()");
    m_itemsOwned.clear();
    m_itemsToBuySell.clear();
    Load();
}

////////////////////////////////////////////////////////////////////////////////

void
Character_t::
Load()
{
    if (!m_itemsOwned.empty() || !m_itemsToBuySell.empty())
    {
        throw logic_error("Character_t::Load(): Already loaded");
    }
    ItemsOwned_t::Load(GetId(), m_itemsOwned);
    ItemsToBuySell_t::Load(GetId(), m_itemsToBuySell);
}

////////////////////////////////////////////////////////////////////////////////

void
Character_t::
DumpItemsOwned() const
{
    m_itemsOwned.Dump();
    m_buyPendingItems.Dump();
}

////////////////////////////////////////////////////////////////////////////////

void
Character_t::
BuySellDump() const
{
    size_t total = 0;
    ItemBuySellMap_t::const_iterator it = m_itemsToBuySell.begin();
    LogAlways(L"Items to buy/sell");
    for (; m_itemsToBuySell.end() != it; ++it)
    {
        const BuySellData_t& data = it->second;
        LogAlways(L"  %3d x (%5d) @ (%4d) - (%-40s)", data.maxToOwn, it->first,
                  data.highBid, Items_t::GetItemName(it->first));
        total += data.maxToOwn;
    }
    LogAlways(L"Unique(%d) Total(%d)", m_itemsToBuySell.size(), total);
}

////////////////////////////////////////////////////////////////////////////////

CDatabase&
Character_t::
GetDb() const
{
    typedef map<ServerId_t, shared_ptr<CDatabase> > ServerDatabaseMap_t;
    static ServerDatabaseMap_t dbMap;

    ServerDatabaseMap_t::iterator it = dbMap.find(m_serverId);
    if (dbMap.end() == it)
    {
        shared_ptr<CDatabase> spDb(new CDatabase);
        OpenDb(*spDb.get());
        auto [elem, inserted] = dbMap.insert(make_pair(m_serverId, spDb));
        if (!inserted)
        {
            throw logic_error("Character_t::GetDb(): dbMap.insert() failed");
        }
        it = elem;
    }
    return *it->second.get();
}

////////////////////////////////////////////////////////////////////////////////

const wstring&
FormatConnectString(
          wstring& dbConnect,
    const wstring& dbPath)
{
    static const wchar_t connectPrefix[] =
        L"DSN=MS Access Database;DBQ=";
    static const wchar_t connectSuffix[] =
        L";DriverId=25;FIL=MS Access;MaxBufferSize=2048;PageTimeout=5;UID=admin;";

    dbConnect.assign(connectPrefix);
    dbConnect.append(dbPath);
    dbConnect.append(connectSuffix);
    return dbConnect;
}

////////////////////////////////////////////////////////////////////////////////
// NOTE: belongs elsewhere
const wstring&
GetServerName(
    ServerId_t serverId)
{
    static wstring mm(L"Mistmoore");
    if (0 != serverId)
    {
        throw invalid_argument("GetServerName()");
    }
    return mm;
}

////////////////////////////////////////////////////////////////////////////////

const wstring&
Character_t::
GetDbPath(
    wstring& dbPath) const
{
    static const wchar_t dbPathPrefix[] = L"\\db\\buysell_";
    dbPath.assign(dbPathPrefix);
    dbPath.append(GetServerName());
    dbPath.append(L".mdb");
    return dbPath;
}

////////////////////////////////////////////////////////////////////////////////

const wstring&
Character_t::
GetDbConnect(
    wstring& dbConnect) const
{
    wstring dbPath;
    return FormatConnectString(dbConnect, GetDbPath(dbPath));
}

///////////////////////////////////////////////////////////////////////////////

void
Character_t::
OpenDb(
   CDatabase& db) const
{
    DWORD Flags = CDatabase::noOdbcDialog;
    wstring connect;
    if (!db.OpenEx(GetDbConnect(connect).c_str(), Flags))
    {
        throw logic_error("Character_t::OpenDb(): OpenEx failed");
    }
}

///////////////////////////////////////////////////////////////////////////////

size_t
Character_t::
WantToBuyHowManyAt(
    const wstring& itemName,
          size_t   price) const
{
    ItemId_t itemId = Items_t::GetItemId(itemName.c_str());
    // if item is in list of items to buy
    ItemBuySellMap_t::const_iterator it = m_itemsToBuySell.find(itemId);
    if (m_itemsToBuySell.end() != it)
    {
        // if we want to own some and price is good
        const BuySellData_t& data = it->second;
        if (0 < data.maxToOwn && data.highBid > price)
        {
            // see if we own any already, or have pending buys
            size_t owned = m_itemsOwned.GetQuantity(itemId) + 
                           m_buyPendingItems.GetQuantity(itemId);
            if (data.maxToOwn > owned)
            {
                return data.maxToOwn - owned;
            }
        }
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

void
Character_t::
InitItemsOwnedFromItemsForSale()
{
    m_itemsOwned.clear();
    ItemForSaleMap_t::const_iterator it = m_itemsForSale.begin();
    for (; m_itemsForSale.end() != it; ++it)
    {
        const ForSaleData_t& data = it->second;
        auto [elem, inserted] = m_itemsOwned.insert(
            make_pair(it->first, data.Quantity));
        if (!inserted)
        {
            elem->second += data.Quantity;
        }
    }
    ItemsOwned_t::Write(GetId(), m_itemsOwned);
}

///////////////////////////////////////////////////////////////////////////////

bool
Character_t::
BuyItem(
    const wstring& itemName,
          size_t   price,
          size_t   quantity /*= 1*/,
          bool     pending /*= false*/)
{
    LogAlways(L"Character_t::BuyItem() %d x (%s) @ %d %s",
              quantity, itemName.c_str(), price, pending ? L"PENDING" : L"");
    ItemId_t itemId = Items_t::GetItemId(itemName.c_str());
    if (0 != itemId)
    {
        AdjustQuantity(itemId, long(quantity), pending);
        return true;
    }
    LogError(L"Character_t::BuyItem() Unknown item (%s)", itemName.c_str());
    return false;
}

///////////////////////////////////////////////////////////////////////////////

void
Character_t::
AdjustQuantity(
    ItemId_t itemId,
    long     quantity,
    bool     pending)
{
    if (pending)
    {
        if (0 < quantity)
        {
            m_buyPendingItems.AdjustQuantity(itemId, quantity);
        }
        else
        {
            throw logic_error("Character_t::AdjustQuantity() pending sales not allowed");
        }
    }
    else
    {
        if (0 < quantity)
        {
            m_buyPendingItems.AdjustQuantity(itemId, -quantity, false);
        }
        m_itemsOwned.AdjustQuantity(itemId, quantity);
        using namespace Accounts::Db;
        ItemsOwned_t::AdjustQuantity(GetId(), itemId, quantity);
    }
}

///////////////////////////////////////////////////////////////////////////////

void
Character_t::
BuySellAdd(
          ItemId_t       itemId,
    const BuySellData_t& data)
{
    if (0 != itemId)
    {
        auto [elem, inserted] = m_itemsToBuySell.insert(make_pair(itemId, data));
        if (inserted)
        {
            LogAlways(L"BuySell::Add() Buy %d x (%d) @ (%d)", 
                      data.maxToOwn, itemId, data.highBid); //GetCoinString(data.highBid));
        }
        else
        {
            const BuySellData_t& extant = elem->second;
            LogError(L"BuySell::Add() Item(%d) already in buySell @ %d x (%s)",
                     itemId, extant.maxToOwn, extant.highBid); // GetCoinString(extant.highBid));
        }
    }
    else
    {
        throw invalid_argument("BuySell::Add()");
    }
}

///////////////////////////////////////////////////////////////////////////////

void
Character_t::
BuySellDelete(
    ItemId_t itemId)
{
    if (0 != itemId)
    {
        ItemBuySellMap_t::iterator it = m_itemsToBuySell.find(itemId);
        if (m_itemsToBuySell.end() != it)
        {
            m_itemsToBuySell.erase(it);
            LogAlways(L"BuySell::Delete() Deleted(%d)", itemId);
        }
        else
        {
            LogError(L"BuySell::Delete() Item(%d) not found", itemId);
        }
    }
    else
    {
        throw invalid_argument("BuySell::Delete()");
    }
}

///////////////////////////////////////////////////////////////////////////////
