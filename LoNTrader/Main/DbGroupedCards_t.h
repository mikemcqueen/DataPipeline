//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// groupedcards_t.h : Declaration of the GroupedCards_t

#pragma once

// code generated on Friday, October 12, 2007, 4:49 PM

class GroupedCards_t : public CRecordset
{
public:
	GroupedCards_t(CDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(GroupedCards_t)

// Field/Param Data

// The string types below (if present) reflect the actual data type of the
// database field - CStringA for ANSI datatypes and CStringW for Unicode
// datatypes. This is to prevent the ODBC driver from performing potentially
// unnecessary conversions.  If you wish, you may change these members to
// CString types and the ODBC driver will perform all necessary conversions.
// (Note: You must use an ODBC driver version that is version 3.5 or greater
// to support both Unicode and these conversions).

	long	m_groupid;
	long	m_cardid;
	long	m_quantity;

// Overrides
	// Wizard generated virtual function overrides
	public:
	virtual CString GetDefaultConnect();	// Default connection string

	virtual CString GetDefaultSQL(); 	// default SQL for Recordset
	virtual void DoFieldExchange(CFieldExchange* pFX);	// RFX support

// Implementation
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

};


