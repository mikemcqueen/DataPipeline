///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// DbTrades_t.h : Implementation of the DbTrades_t class
//
///////////////////////////////////////////////////////////////////////////////



// DbTrades_t implementation

// code generated on Friday, October 12, 2007, 4:51 PM

#include "stdafx.h"
#include "DbTrades_t.h"
#include "LonTrader_t.h"

IMPLEMENT_DYNAMIC(DbTrades_t, CRecordset)
//IMPLEMENT_DYNAMIC(DbRemovedTrades_t, CRecordset)
//IMPLEMENT_DYNAMIC(DbOtherTrades_t, CRecordset)

DbTrades_t::DbTrades_t(CDatabase* pdb)
	: CRecordset(pdb)
{
	m_tradeid = 0;
	m_username = L"";
	m_offerid = 0;
	m_wantid = 0;
	m_offervalue = 0;
	m_wantvalue = 0;
    m_flags = 0;
    m_postedtime;
    m_removedtime;
	m_nFields = 9;
	m_nDefaultType = snapshot;
}
CString DbTrades_t::GetDefaultConnect()
{
    return LonTrader_t::GetDbConnectString();
//	return _T("DSN=MS Access Database;DBQ=U:\\lonbackup.mdb;DefaultDir=U:;DriverId=25;FIL=MS Access;MaxBufferSize=2048;PageTimeout=5;UID=admin;");
}

CString DbTrades_t::GetDefaultSQL()
{
	return _T("[trades_t]");
}

void DbTrades_t::DoFieldExchange(CFieldExchange* pFX)
{
	pFX->SetFieldType(CFieldExchange::outputColumn);
// Macros such as RFX_Text() and RFX_Int() are dependent on the
// type of the member variable, not the type of the field in the database.
// ODBC will try to automatically convert the column value to the requested type
	RFX_Long(pFX, _T("[tradeid]"),    m_tradeid);
	RFX_Text(pFX, _T("[username]"),   m_username);
	RFX_Long(pFX, _T("[offerid]"),    m_offerid);
	RFX_Long(pFX, _T("[wantid]"),     m_wantid);
	RFX_Long(pFX, _T("[offervalue]"), m_offervalue);
	RFX_Long(pFX, _T("[wantvalue]"),  m_wantvalue);
	RFX_Long(pFX, _T("[flags]"),      m_flags);
    RFX_Date(pFX, _T("[postedtime]"), m_postedtime);
    RFX_Date(pFX, _T("[removedtime]"),m_removedtime);
}
/////////////////////////////////////////////////////////////////////////////
// DbTrades_t diagnostics

#ifdef _DEBUG
void DbTrades_t::AssertValid() const
{
	CRecordset::AssertValid();
}

void DbTrades_t::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG


