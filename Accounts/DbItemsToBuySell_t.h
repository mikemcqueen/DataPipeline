////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// DbItemsToBuySell_t.h
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Recordset_t.h"
#include "GameId.h"

////////////////////////////////////////////////////////////////////////////////

struct BuySellData_t;
typedef map<ItemId_t, BuySellData_t> ItemBuySellMap_t;

namespace Accounts
{
namespace Db
{

    class ItemsToBuySell_t :
        public Recordset_t
    {
    public:

        struct Field
        {
            enum : Recordset_t::Field_t
            {
                ItemId             = 0x00001,
                LowBid             = 0x00002,
                HighBid            = 0x00004,
                BidIncrement       = 0x00008,
                LowAsk             = 0x00010,
                HighAsk            = 0x00020,
                AskIncrement       = 0x00040,
                MaxToOwn           = 0x00080,
                MaxToSell          = 0x00100,
                MaxForSale         = 0x00200,
                ServerId           = 0x00400,
                CharacterId        = 0x00800,
                AllFields          = 0x0ffff,

                CharacterIdParam   = 0x10000,
                ServerIdParam      = 0x20000,
            };
        };

        long	m_ItemId;
        long	m_LowBid;
        long    m_HighBid;
        long	m_BidIncrement;
        long	m_LowAsk;
        long    m_HighAsk;
        long	m_AskIncrement;
        long	m_MaxToOwn;
        long	m_MaxToSell;
        long	m_MaxForSale;
        long	m_ServerId;
        long	m_CharacterId;

        long	m_CharacterIdParam;
        long	m_ServerIdParam;

    public:

        ItemsToBuySell_t(
            CDatabase* pDatabase = nullptr,
            bool       bAllowDefaultConnect = false);

        DECLARE_DYNAMIC(ItemsToBuySell_t)

        virtual CString GetDefaultConnect();
        virtual CString GetDefaultSQL();
        virtual void DoFieldExchange(CFieldExchange* pFX);

        static
        void
        Load(
            CharacterId_t     id,
            ItemBuySellMap_t& map);

    private:

        static
        void
        Load(
            CDatabase&        db,
            CharacterId_t     id,
            ItemBuySellMap_t& map);

#ifdef _DEBUG
        virtual void AssertValid() const;
        virtual void Dump(CDumpContext& dc) const;
#endif
    };

} // Db
} // Accounts

struct BuySellData_t
{
    BuySellData_t()
    {
        SecureZeroMemory(this, sizeof(BuySellData_t));
    }
    BuySellData_t(const Accounts::Db::ItemsToBuySell_t& rs);

    size_t lowBid;
    size_t highBid;
    size_t incBid;
    size_t lowAsk;
    size_t highAsk;
    size_t incAsk;
    size_t maxToOwn;
    size_t maxToSell;
    size_t maxForSale;
};
//typedef map<ItemId_t, BuySellData_t> ItemBuySellMap_t;

