////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// DbAccounts_t.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DbAccounts_t.h"


IMPLEMENT_DYNAMIC(DbAccounts_t, CRecordset)

DbAccounts_t::
DbAccounts_t(
    CDatabase* pdb,
    bool bAllowDefaultConnect)
:
    Recordset_t(pdb, bAllowDefaultConnect)
{
	m_ItemId = 0;
	m_SellPriceMin = 0;
	m_SellPriceMax = 0;
	m_SellPriceIncrement = 0;
	m_QuantityOwned = 0;
	m_MaxToOwn = 0;
	m_MaxToSell = 0;

	m_nDefaultType = dynaset;
}

CString DbAccounts_t::GetDefaultConnect()
{
    if (!AllowDefaultConnect())
    {
        throw std::logic_error("DbAccounts_t::GetDefaultConnect() not allowed");
    }
    throw logic_error("DbAccounts_t not impelemented");
//    return GetConnectString(BuySell);
}

CString DbAccounts_t::GetDefaultSQL()
{
	return _T("[items_to_sell]");
}

void DbAccounts_t::DoFieldExchange(CFieldExchange* pFX)
{
	pFX->SetFieldType(CFieldExchange::outputColumn);
    if (CheckField(Field::ItemId))
    {
        RFX_Long(pFX, _T("[item_id]"), m_ItemId);
    }
    if (CheckField(Field::SellPriceMin))
    {
        RFX_Long(pFX, _T("[sell_price_min]"), m_SellPriceMin);
    }
    if (CheckField(Field::SellPriceMax))
    {
        RFX_Long(pFX, _T("[sell_price_max]"), m_SellPriceMax);
    }
    if (CheckField(Field::SellPriceIncrement))
    {
        RFX_Long(pFX, _T("[sell_price_increment]"), m_SellPriceIncrement);
    }
    if (CheckField(Field::QuantityOwned))
    {
        RFX_Long(pFX, _T("[quantity_owned]"), m_QuantityOwned);
    }
    if (CheckField(Field::MaxToOwn))
    {
        RFX_Long(pFX, _T("[max_to_own]"), m_MaxToOwn);
    }
    if (CheckField(Field::SellPriceMin))
    {
        RFX_Long(pFX, _T("[max_to_sell]"), m_MaxToSell);
    }
}

////////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
void DbAccounts_t::AssertValid() const
{
	CRecordset::AssertValid();
}

void DbAccounts_t::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG

////////////////////////////////////////////////////////////////////////////////

