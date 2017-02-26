
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// Items_t.h

#pragma once

#include "Recordset_t.h"
#include "GameId.h"

namespace Accounts
{
namespace Db
{

static const size_t kItemNameMax = 100;

class Items_t :
    public Recordset_t
{

public:

    struct Field
    {
        enum Type_e : Recordset_t::Field_t
        {
            ItemId       = 0x0001,
            ItemName     = 0x0002,
            ItemFlags    = 0x0004,
            AllFields    = 0x00ff,

            StringParam1 = 0x0100,
            ItemIdParam  = 0x0200,
        };
    };

    // Field/Param Data

    long      m_item_id;
	CStringW  m_item_name;
	long      m_item_flags;

    CStringW  m_strParam1;
    long      m_ItemIdParam;

    // Constructor:

	Items_t(CDatabase* pDatabase = NULL);

    explicit
    Items_t(CDatabase* pdb, const wchar_t* pParam);

    void Init();


	DECLARE_DYNAMIC(Items_t)


// Overrides
	public:
	virtual CString GetDefaultConnect();	// Default connection string

	virtual CString GetDefaultSQL(); 	// default SQL for Recordset
	virtual void DoFieldExchange(CFieldExchange* pFX);	// RFX support


// Implementation

    static
    ItemId_t
    GetItemId(
        const wchar_t* pItemName);

    static
    const wchar_t*
    GetItemName(
        ItemId_t itemId);

    static
    const wchar_t*
    GetItemName(
        CDatabase& Db,
        ItemId_t   ItemId,
        wchar_t*   pName,
        size_t     NameCount);

    static
    ItemId_t
    AddItem(
        const wchar_t* pItemName,
              long     Flags = 0);

private:

    static
    ItemId_t
    GetItemId(
              CDatabase& db,
        const wchar_t*   pItemName);

    static
    ItemId_t
    AddItem(
              CDatabase& db,
        const wchar_t*   pItemName,
              long       Flags = 0);

public:

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
};
 
} // Db
} // Accounts
