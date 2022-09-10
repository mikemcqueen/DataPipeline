////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// DbItemsOwned_t.h
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Recordset_t.h"
#include "GameId.h"
#include "AccountsDb.h"

class ItemQuantityMap_t;

namespace Accounts
{
namespace Db
{

    class ItemsOwned_t :
        public Recordset_t
    {
    public:

        struct Field
        {
            enum : Recordset_t::Field_t
            {
                ItemId             = 0x00001,
                Quantity           = 0x00002,
                CharacterId        = 0x00004,
                AllFields          = 0x0ffff,
                ItemIdParam        = 0x10000,
                CharacterIdParam   = 0x20000,
            };
        };

        long m_item_id;
        long m_quantity;
        long m_character_id;

        long m_ItemIdParam;
        long m_CharacterIdParam;

    public:

        ItemsOwned_t(
            CDatabase* pDatabase = nullptr,
            bool       bAllowDefaultConnect = false);

        DECLARE_DYNAMIC(ItemsOwned_t)

        virtual CString GetDefaultConnect();
        virtual CString GetDefaultSQL();
        virtual void DoFieldExchange(CFieldExchange* pFX);

        static
        void
        Load(
            ItemId_t           itemId,
            ItemQuantityMap_t& map);

        static
        size_t
        Write(
                  CharacterId_t      characterId,
            const ItemQuantityMap_t& map);

        static
        void
        AdjustQuantity(
            CharacterId_t characterId,
            ItemId_t      itemId,
            long          quantity);

    private:

        static
        void
        Load(
            CDatabase&         db,
            ItemId_t           itemId,
            ItemQuantityMap_t& map);

        static
        size_t
        Write(
                  CDatabase&         db,
                  CharacterId_t      characterId,
            const ItemQuantityMap_t& map);

#ifdef _DEBUG
        virtual void AssertValid() const;
        virtual void Dump(CDumpContext& dc) const;
#endif
    };

} // Db
} // Accounts