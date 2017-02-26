
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//

// DbCardSet_t.h : Implementation of the DbCardSet_t class



// DbCardSet_t implementation

// code generated on Monday, January 21, 2008, 3:33 AM

#include "stdafx.h"
#include "DbCardSet_t.h"
#include "LonTrader_t.h"

IMPLEMENT_DYNAMIC(DbCardSet_t, CRecordset)

DbCardSet_t::DbCardSet_t(CDatabase* pdb)
	: CRecordset(pdb)
{
	m_cardid = 0;
	m_quantity = 0;
	m_nFields = 2;
    m_nDefaultType = snapshot;
}

CString DbCardSet_t::GetDefaultConnect()
{
    return LonTrader_t::GetDbConnectString();
}

CString DbCardSet_t::GetDefaultSQL()
{
	return _T("[yourcards_t]");
}

void DbCardSet_t::DoFieldExchange(CFieldExchange* pFX)
{
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Long(pFX, _T("[cardid]"), m_cardid);
	RFX_Long(pFX, _T("[quantity]"), m_quantity);

}
/////////////////////////////////////////////////////////////////////////////
// DbCardSet_t diagnostics

#ifdef _DEBUG
void DbCardSet_t::AssertValid() const
{
	CRecordset::AssertValid();
}

void DbCardSet_t::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG


