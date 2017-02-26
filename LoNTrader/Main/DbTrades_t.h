//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// dbtrades_t.h : Declaration of the DbTrades_t

#pragma once

// code generated on Friday, October 12, 2007, 4:51 PM

class DbTrades_t : public CRecordset
{
public:
	DbTrades_t(CDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(DbTrades_t)

// Field/Param Data

// The string types below (if present) reflect the actual data type of the
// database field - CStringA for ANSI datatypes and CStringW for Unicode
// datatypes. This is to prevent the ODBC driver from performing potentially
// unnecessary conversions.  If you wish, you may change these members to
// CString types and the ODBC driver will perform all necessary conversions.
// (Note: You must use an ODBC driver version that is version 3.5 or greater
// to support both Unicode and these conversions).

	long	  m_tradeid;
	CStringW  m_username;
	long	  m_offerid;
	long	  m_wantid;
	long	  m_offervalue;
	long	  m_wantvalue;
    long      m_flags;
    CTime     m_postedtime;
    CTime     m_removedtime;

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
/*
class DbRemovedTrades_t :
    public DbTrades_t
{
public:
    DbRemovedTrades_t(CDatabase* pDatabase = NULL) : DbTrades_t(pDatabase) {}
	DECLARE_DYNAMIC(DbRemovedTrades_t)

	virtual CString GetDefaultSQL() 	// default SQL for Recordset
    {
        return _T("[removedtrades_t]");
    }
};

class DbOtherTrades_t :
    public DbTrades_t
{
public:
    DbOtherTrades_t(CDatabase* pDatabase = NULL) : DbTrades_t(pDatabase) {}
	DECLARE_DYNAMIC(DbOtherTrades_t)

	virtual CString GetDefaultSQL()
    {
        return _T("[othertrades_t]");
    }

};

*/