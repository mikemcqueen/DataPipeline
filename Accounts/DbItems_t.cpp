// Items_t.h : Implementation of the Items_t class

#include "stdafx.h"
#include "DbItems_t.h"
#include "Log.h"
#include "Macros.h"
#include "AccountsDb.h"

namespace Accounts
{
namespace Db
{

IMPLEMENT_DYNAMIC(Items_t, CRecordset)

Items_t::Items_t(CDatabase* pdb)
	: Recordset_t(pdb, false)
{
    Init();
}

Items_t::
Items_t(
    CDatabase* pdb,
    const wchar_t* pParam)
:
    Recordset_t(pdb, false)
{
    Init();
    m_strParam1 = pParam;
}

void
Items_t::
Init()
{
	m_item_id = 0;
	m_item_name = L"";
	m_item_flags = 0;

    m_ItemIdParam = 0;

	m_nDefaultType = dynaset;
}

CString Items_t::GetDefaultConnect()
{
    return GetConnectString(Items);
}

CString Items_t::GetDefaultSQL()
{
	return L"[items]";
}

void Items_t::
DoFieldExchange(
    CFieldExchange* pFX)
{
	pFX->SetFieldType(CFieldExchange::outputColumn);
    if (CheckField(Field::ItemId))
    {
        RFX_Long(pFX, _T("[item_id]"), m_item_id);
    }
    if (CheckField(Field::ItemName))
    {
        RFX_Text(pFX, _T("[item_name]"), m_item_name);
    }
    if (CheckField(Field::ItemFlags))
    {
        RFX_Long(pFX, _T("[item_flags]"), m_item_flags);
    }

    pFX->SetFieldType(CFieldExchange::param);
    if (CheckField(Field::StringParam1))
    {
        RFX_Text(pFX, _T("param1"), m_strParam1);
    }
    if (CheckField(Field::ItemIdParam))
    {
        RFX_Long(pFX, _T("[item_id_param]"), m_ItemIdParam);
    }
}

////////////////////////////////////////////////////////////////////////////////

/*static*/
ItemId_t
Items_t::
GetItemId(
    const wchar_t* pItemName)
{
    if ((nullptr == pItemName) || (L'\0' == *pItemName))
    {
        throw std::invalid_argument("Items_t::GetItemId()");
    }
    ItemId_t ItemId = 0;
    try
    {
        ItemId = GetItemId(GetDb(Items), pItemName);
    }
    catch (CDBException* e)
    {
        LogError(L"Items_t::GetItemId() exception: %ls", (LPCTSTR)e->m_strError);
        e->Delete();
    }
    return ItemId;
}

////////////////////////////////////////////////////////////////////////////////

ItemId_t
Items_t::
GetItemId(
          CDatabase& db,
    const wchar_t*   pItemName)
{
    Items_t rs(&db, pItemName);
//    static const wchar_t szSql[] = 
//        L"SELECT item_id FROM items WHERE item_name = ?";
    rs.AddField(Field::ItemId);
    rs.m_strFilter = L"[item_name] = ?";
    rs.AddParam(Field::StringParam1);
    if (0 == rs.Open(CRecordset::forwardOnly, nullptr, DefaultReadOnlyFlags))
    {
        LogError(L"Items_t::GetItemId(): failed at rs.Open()");
        return false;
    }
    return rs.IsEOF() ? 0 : rs.m_item_id;
}

////////////////////////////////////////////////////////////////////////////////

/*static*/
const wchar_t*
Items_t::
GetItemName(
    ItemId_t itemId)
{
    static wchar_t itemName[kItemNameMax + 1];
    itemName[0] = L'\0';
    try
    {
        return GetItemName(GetDb(Items), itemId, itemName, _countof(itemName));
    }
    catch (CDBException* e)
    {
        LogError(L"Items_t::GetItemName() Id(%d) exception: %ls", itemId, (LPCTSTR)e->m_strError);
        e->Delete();
    }
    return nullptr;
}


////////////////////////////////////////////////////////////////////////////////

/*static*/
const wchar_t*
Items_t::
GetItemName(
    CDatabase& db,
    ItemId_t ItemId,
    wchar_t* pName,
    size_t   NameCount)
{
    //TODO: AssertArg((nullptr != pName) && (0 < NameCount))
    if ((nullptr == pName) || (0 == NameCount))
    {
        throw std::invalid_argument("ItemsId_t::GetItemName()");
    }
    Items_t rs(&db);
    rs.AddField(Field::ItemName);
    rs.m_strFilter = L"[item_id] = ?";
    rs.m_ItemIdParam = ItemId;
    rs.AddParam(Field::ItemIdParam);
    if (0 == rs.Open(CRecordset::forwardOnly, nullptr, DefaultReadOnlyFlags))
    {
        LogError(L"Items_t::GetItemName(): rs.Open() failed");
        return nullptr;
    }
    if (rs.IsEOF())
    {
        return nullptr;
    }
    wcscpy_s(pName, NameCount, rs.m_item_name);
    return pName;
}

////////////////////////////////////////////////////////////////////////////////

ItemId_t
Items_t::
AddItem(
    const wchar_t* pItemName,
          long     Flags)
{
    ItemId_t ItemId = 0;
    try
    {
        CDatabase& db = GetDb(Items);
        ItemId = GetItemId(db, pItemName);
        if (0 == ItemId)
        {
            ItemId = AddItem(db, pItemName, Flags);
        }
    }
    catch (CDBException* e)
    {
        LogError(L"Items_t::AddItem(%ls) exception: %ls",
                 pItemName, (LPCTSTR)e->m_strError);
        e->Delete();
    }
    return ItemId;
}

////////////////////////////////////////////////////////////////////////////////

ItemId_t
Items_t::
AddItem(
          CDatabase& db,
    const wchar_t*   pItemName,
          long       Flags)
{
    Items_t rs(&db);
    rs.AddField(Field::ItemId);
    rs.AddField(Field::ItemName);
    rs.AddField(Field::ItemFlags);
    rs.m_strFilter = L"1 = 0";
    if (0 == rs.Open(CRecordset::dynaset, nullptr, CRecordset::appendOnly))
    {
        LogError(L"Items_t::AddItem(): failed at rs.Open()");
        return 0;
    }
    if (!rs.CanAppend())
    {
        LogError(L"Items_t::AddItem(): failed at !rs.CanAppend()");
        return 0;
    }
    rs.AddNew();
    rs.m_item_name  = pItemName;
    rs.m_item_flags = Flags;
    if (0 == rs.Update())
    {
        LogError(L"Items_t::AddItem(): failed at rs.Update()");
        return 0;
    }
    rs.m_strFilter = L"";
    if (0 == rs.Requery())
    {
        LogError(L"Items_t::AddItem(): failed at rs.Requery()");
        return 0;
    }
    rs.SetAbsolutePosition(-1);
    LogAlways(L"Added '%ls' to DB (%d)", (LPCTSTR)rs.m_item_name, rs.m_item_id);
    rs.Close();
    return rs.m_item_id;
}

/////////////////////////////////////////////////////////////////////////////
// Items_t diagnostics

#ifdef _DEBUG
void Items_t::AssertValid() const
{
	CRecordset::AssertValid();
}

void Items_t::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG

} // Db
} // Accounts