////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// Character_t.h
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "DbItemsToBuySell_t.h"
#include "ItemQuantityMap_t.h"
#include "GameId.h"

////////////////////////////////////////////////////////////////////////////////

typedef unsigned CharacterId_t;

////////////////////////////////////////////////////////////////////////////////
// EQ2 broker specific
struct ForSaleData_t
{
    size_t Quantity;
    size_t Price;
    bool   isListed;
};
typedef std::multimap<ItemId_t, ForSaleData_t> ItemForSaleMap_t;

////////////////////////////////////////////////////////////////////////////////

const std::wstring&
GetServerName(
    ServerId_t serverId);

class Character_t
{
    friend class Eq2Broker_t;

private:

    const Account_t&  m_account;
    ServerId_t        m_serverId;
    CharacterId_t     m_id;
    std::wstring           m_name;

    
    ItemQuantityMap_t m_itemsOwned;
    ItemQuantityMap_t m_buyPendingItems;  // a transaction to buy has been queued for these
    ItemBuySellMap_t  m_itemsToBuySell;

    // determined live, maybe save copy in db?
    ItemForSaleMap_t  m_itemsForSale;

public:

    // Static functions:

    static
    void
    SetCharacter(
        Character_t* pChar);

    static
    Character_t&
    GetCharacter();

public:

    Character_t(
        const Account_t& account,
              ServerId_t serverId,
        CharacterId_t    Id,
        const wchar_t*   pName)
    :
        m_itemsOwned(L"OwnedItems"),
        m_buyPendingItems(L"BuyPendingItems"),
        m_account(account),
        m_serverId(serverId),
        m_id(Id),
        m_name(pName)
    {
    }

    ItemQuantityMap_t& GetItemsOwned()      { return m_itemsOwned; }
    ItemForSaleMap_t&  GetItemsForSale()    { return m_itemsForSale; }
    ItemBuySellMap_t&  GetItemsToBuySell()  { return m_itemsToBuySell; }

    void Load();
    void Reload();

    const CharacterId_t GetId() const         { return m_id; };
    const std::wstring& GetName() const       { return m_name; }
    const std::wstring& GetServerName() const { return ::GetServerName(m_serverId); }

    size_t
    WantToBuyHowManyAt(
        const std::wstring& itemName,
        size_t price) const;

    void
    InitItemsOwnedFromItemsForSale();

    bool
    BuyItem(
        const std::wstring& itemName,
              size_t   price,
              size_t   quantity = 1,
              bool     pending = false);

    void
    BuySellAdd(
              ItemId_t       itemId,
        const BuySellData_t& data);

    void
    BuySellDelete(
        ItemId_t itemId);

    void
    BuySellDump() const;

#if 0
    void
    BuySellModify(
              ItemId_t       itemId,
        const BuySellData_t& data);
#endif

private:

    void
    AdjustQuantity(
        ItemId_t itemId,
        long     quantity,
        bool     pending);

    void
    DbAdjustQuantity(
        ItemId_t itemId,
        long     quantity);

    void
    LoadItemsOwned();

    void
    LoadItemsOwned(
        CDatabase&         db,
        ItemQuantityMap_t& map);

    size_t
    WriteItemsOwned() const;

    size_t
    WriteItemsOwned(
              CDatabase&         db,
        const ItemQuantityMap_t& map) const;

    void
    DumpItemsOwned() const;

    void
    LoadItemsToBuySell(
        CDatabase&        db,
        ItemBuySellMap_t& map);

    CDatabase&
    GetDb() const;

    void
    OpenDb(
        CDatabase& db) const;

    const std::wstring&
    GetDbPath(
        std::wstring& dbPath) const;

    const std::wstring&
    GetDbConnect(
        std::wstring& dbConnect) const;

private:

    Character_t();
    Character_t(const Character_t&);
    Character_t& operator=(const Character_t&);
};

Character_t& GetCharacter();
