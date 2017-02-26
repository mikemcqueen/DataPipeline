
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//

// groupedcards_t.h : Implementation of the GroupedCards_t class



// GroupedCards_t implementation

// code generated on Friday, October 12, 2007, 4:49 PM

#include "stdafx.h"
#include "DbGroupedCards_t.h"
#include "LonTrader_t.h"

IMPLEMENT_DYNAMIC(GroupedCards_t, CRecordset)

GroupedCards_t::GroupedCards_t(CDatabase* pdb)
	: CRecordset(pdb)
{
	m_groupid = 0;
	m_cardid = 0;
	m_quantity = 0;
	m_nFields = 3;
	m_nDefaultType = snapshot;
}

CString GroupedCards_t::GetDefaultConnect()
{
    return LonTrader_t::GetDbConnectString();
}

CString GroupedCards_t::GetDefaultSQL()
{
	return _T("[groupedcards_t]");
}

void GroupedCards_t::DoFieldExchange(CFieldExchange* pFX)
{
	pFX->SetFieldType(CFieldExchange::outputColumn);
	RFX_Long(pFX, _T("[groupid]"), m_groupid);
	RFX_Long(pFX, _T("[cardid]"), m_cardid);
	RFX_Long(pFX, _T("[quantity]"), m_quantity);
}

/////////////////////////////////////////////////////////////////////////////
// GroupedCards_t diagnostics

#ifdef _DEBUG
void GroupedCards_t::AssertValid() const
{
	CRecordset::AssertValid();
}

void GroupedCards_t::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG


