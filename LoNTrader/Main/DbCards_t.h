//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//

// dbcards_t.h : Declaration of the DbCards_t

#pragma once

// code generated on Thursday, October 18, 2007, 5:46 AM

class DbCards_t : public CRecordset
{
public:
	DbCards_t(CDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(DbCards_t)

// Field/Param Data

// The string types below (if present) reflect the actual data type of the
// database field - CStringA for ANSI datatypes and CStringW for Unicode
// datatypes. This is to prevent the ODBC driver from performing potentially
// unnecessary conversions.  If you wish, you may change these members to
// CString types and the ODBC driver will perform all necessary conversions.
// (Note: You must use an ODBC driver version that is version 3.5 or greater
// to support both Unicode and these conversions).

	long	m_cardid;
	CStringW	m_cardname;
	long	m_value;
	long	m_buyat;
	long	m_sellat;
	long	m_buyingat;
	long	m_sellingat;

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


