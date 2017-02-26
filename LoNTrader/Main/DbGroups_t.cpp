
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//

// groups_t.h : Implementation of the Groups_t class



// Groups_t implementation

// code generated on Friday, October 12, 2007, 4:46 PM

#include "stdafx.h"
#include "DbGroups_t.h"
#include "LonTrader_t.h"

IMPLEMENT_DYNAMIC(Groups_t, CRecordset)

Groups_t::Groups_t(CDatabase* pdb)
	: CRecordset(pdb)
{
	m_groupid = 0;
	m_value = 0;
	m_nFields = 2;
	m_nDefaultType = snapshot;
}

CString Groups_t::GetDefaultConnect()
{
    return LonTrader_t::GetDbConnectString();
}

CString Groups_t::GetDefaultSQL()
{
	return _T("[groups_t]");
}

void Groups_t::DoFieldExchange(CFieldExchange* pFX)
{
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Long(pFX, _T("[groupid]"), m_groupid);
	RFX_Long(pFX, _T("[value]"), m_value);
}

/////////////////////////////////////////////////////////////////////////////
// Groups_t diagnostics

#ifdef _DEBUG
void Groups_t::AssertValid() const
{
	CRecordset::AssertValid();
}

void Groups_t::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG


