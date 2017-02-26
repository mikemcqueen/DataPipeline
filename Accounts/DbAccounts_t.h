////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// DbAccounts_t.h
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Recordset_t.h"

    class DbAccounts_t :
        public Recordset_t
    {
    public:

        struct Field
        {
            enum : Recordset_t::Field_t
            {
                ItemId             = 0x00001,
                SellPriceMin       = 0x00002,
                SellPriceMax       = 0x00004,
                SellPriceIncrement = 0x00008,
                QuantityOwned      = 0x00010,
                MaxToOwn           = 0x00020,
                MaxToSell          = 0x00040,
                AllFields          = 0x0ffff,
            };
        };

        long	m_ItemId;
        long	m_SellPriceMin;
        long	m_SellPriceMax;
        long	m_SellPriceIncrement;
        long	m_QuantityOwned;
        long	m_MaxToOwn;
        long	m_MaxToSell;

    public:

        DbAccounts_t(
            CDatabase* pDatabase = NULL,
            bool       bAllowDefaultConnect = false);

        DECLARE_DYNAMIC(DbAccounts_t)

        virtual CString GetDefaultConnect();
        virtual CString GetDefaultSQL();
        virtual void DoFieldExchange(CFieldExchange* pFX);

#ifdef _DEBUG
        virtual void AssertValid() const;
        virtual void Dump(CDumpContext& dc) const;
#endif
    };

