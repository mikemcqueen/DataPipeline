// Sellers_t.h : Declaration of the Sellers_t

#pragma once

#include "Recordset_t.h"
#include "GameId.h"

namespace Accounts
{
namespace Db
{

class Sellers_t :
    public Recordset_t
{
public:

    struct Field
    {
        enum Type_e : Recordset_t::Field_t
        {
            SellerId     = 0x0001,
            SellerName   = 0x0002,
            MarketId     = 0x0004,
            AddedDate    = 0x0008,
            VerifiedDate = 0x0010,
            AllFields    = 0x00ff,

            StringParam1 = 0x0100,
        };
    };

	Sellers_t(
        CDatabase* pDatabase = nullptr);

    explicit
    Sellers_t(
        CDatabase* pdb,
        const wchar_t* pParam);

	DECLARE_DYNAMIC(Sellers_t)

    void Init();

// Field/Param Data

	long             m_seller_id;
	CStringW         m_seller_name;
	long             m_market_id;
	TIMESTAMP_STRUCT m_added_date;
    TIMESTAMP_STRUCT m_verified_date;

    CStringW         m_strParam1;

// Overrides
	// Wizard generated virtual function overrides
	public:
	virtual CString GetDefaultConnect();	// Default connection string

	virtual CString GetDefaultSQL(); 	// default SQL for Recordset
	virtual void DoFieldExchange(CFieldExchange* pFX);	// RFX support


// Implementation

    static
    SellerId_t
    GetSellerId(
        const wchar_t* pSellerName);

    static
    SellerId_t
    AddSeller(
        const wchar_t* pSellerName);

private:

    static
    SellerId_t
    GetSellerId(
              CDatabase& db,
        const wchar_t*   pSellerName);

    static
    SellerId_t
    AddSeller(
              CDatabase& db,
        const wchar_t*   pSellerName);

public:

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

};

} // Db
} // Accounts