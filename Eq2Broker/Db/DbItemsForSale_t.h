// ItemsForSale_t.h : Declaration of the ItemsForSale_t

#pragma once

#include "DbSellers_t.h"
#include "DbItems_t.h"
#include "Recordset_t.h"
#include "GameId.h"

namespace Db
{

typedef long SaleId_t;

class ItemsForSale_t :
    public Recordset_t
{

public:

    struct Field
    {
        enum Type_e : Recordset_t::Field_t
        {
            SaleId        = 0x00001,
            ItemId        = 0x00002,
            Price         = 0x00004,
            Quantity      = 0x00008,
            Commission    = 0x00010,
            MarketId      = 0x00020,
            SellerId      = 0x00040,
            AddedDate     = 0x00080,
            VerifiedDate  = 0x00100,
            AllFields     = 0x0ffff,

            ItemIdParam   = 0x10000,
            SellerIdParam = 0x20000,
        };
    };

	long	m_SaleId;
	long	m_ItemId;
	long	m_Price;
	long	m_Quantity;
	long	m_Commission;
	long	m_MarketId;
	long	m_SellerId;
	TIMESTAMP_STRUCT m_AddedDate;
    TIMESTAMP_STRUCT m_VerifiedDate;

    long    m_ItemIdParam;
    long    m_SellerIdParam;

public:

	ItemsForSale_t(
        CDatabase* pDatabase = nullptr,
        bool       bAllowDefaultConnect = false);

	DECLARE_DYNAMIC(ItemsForSale_t)

// Overrides
	// Wizard generated virtual function overrides
	public:
	virtual CString GetDefaultConnect();	// Default connection string

	virtual CString GetDefaultSQL(); 	// default SQL for Recordset
	virtual void DoFieldExchange(CFieldExchange* pFX);	// RFX support

    static
    SaleId_t
    AddItem(
        const wchar_t* pItemName,
              size_t   Price,
              size_t   Quantity,
        const wchar_t* pSellerName);

    static
    SaleId_t
    AddItem(
        ItemId_t   ItemId,
        size_t     Price,
        size_t     Quantity,
        SellerId_t SellerId);

    static
    SaleId_t
    AddItem(
        CDatabase& Db,
        ItemId_t   ItemId,
        size_t     Price,
        size_t     Quantity,
        SellerId_t SellerId);

    static
    size_t
    GetSaleCount(
        CDatabase& Db,
        ItemId_t   ItemId,
        SellerId_t SellerId);

    static
    SaleId_t
    FindItem(
        CDatabase& Db,
        ItemId_t   ItemId,
        SellerId_t SellerId);

// Implementation
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

};

} // Db
