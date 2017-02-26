
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//

// dbcards_t.h : Implementation of the DbCards_t class



// DbCards_t implementation

// code generated on Thursday, October 18, 2007, 5:46 AM

#include "stdafx.h"
#include "dbcards_t.h"
#include "LonTrader_t.h"

IMPLEMENT_DYNAMIC(DbCards_t, CRecordset)

DbCards_t::DbCards_t(CDatabase* pdb)
	: CRecordset(pdb)
{
	m_cardid = 0;
	m_cardname = L"";
	m_value = 0;
	m_buyat = 0;
	m_sellat = 0;
	m_buyingat = 0;
	m_sellingat = 0;
	m_nFields = 7;
	m_nDefaultType = snapshot;
}

CString DbCards_t::GetDefaultConnect()
{
    return LonTrader_t::GetDbConnectString();
//	return _T("DSN=MS Access Database;DBQ=E:\\db\\trades.mdb;DefaultDir=E:\\db;DriverId=25;FIL=MS Access;MaxBufferSize=2048;PageTimeout=5;UID=admin;");
}

CString DbCards_t::GetDefaultSQL()
{
	return _T("[cards_t]");
}

void DbCards_t::DoFieldExchange(CFieldExchange* pFX)
{
	pFX->SetFieldType(CFieldExchange::outputColumn);
// Macros such as RFX_Text() and RFX_Int() are dependent on the
// type of the member variable, not the type of the field in the database.
// ODBC will try to automatically convert the column value to the requested type
	RFX_Long(pFX, _T("[cardid]"), m_cardid);
	RFX_Text(pFX, _T("[cardname]"), m_cardname);
	RFX_Long(pFX, _T("[value]"), m_value);
	RFX_Long(pFX, _T("[buyat]"), m_buyat);
	RFX_Long(pFX, _T("[sellat]"), m_sellat);
	RFX_Long(pFX, _T("[buyingat]"), m_buyingat);
	RFX_Long(pFX, _T("[sellingat]"), m_sellingat);

}
/////////////////////////////////////////////////////////////////////////////
// DbCards_t diagnostics

#ifdef _DEBUG
void DbCards_t::AssertValid() const
{
	CRecordset::AssertValid();
}

void DbCards_t::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG


