// ItemsForSale_t.h : Implementation of the ItemsForSale_t class

#include "stdafx.h"
#include "DbItemsForSale_t.h"
#include "DbSellers_t.h"
#include "DbItems_t.h"
#include "Log.h"
#include "Db.h"
#include "AccountsDb.h"

namespace Db
{

IMPLEMENT_DYNAMIC(ItemsForSale_t, CRecordset)

ItemsForSale_t::
ItemsForSale_t(
    CDatabase* pDb,
    bool bAllowDefaultConnect)
:
    Recordset_t(pDb, bAllowDefaultConnect)
{
	m_SaleId    = 0;
    m_ItemId    = 0;
	m_Price = 0;
	m_Quantity   = 0;
	m_Commission = 0;
	m_MarketId  = 0;
	m_SellerId  = 0;
    SecureZeroMemory(&m_AddedDate, sizeof(m_AddedDate));
    SecureZeroMemory(&m_VerifiedDate, sizeof(m_VerifiedDate));

    m_ItemIdParam   = 0;
    m_SellerIdParam = 0;

	m_nDefaultType = dynaset;

}

////////////////////////////////////////////////////////////////////////////////

CString ItemsForSale_t::GetDefaultConnect()
{
    if (!AllowDefaultConnect())
    {
        throw std::logic_error("DbItemsForSale_t::GetDefaultConnect() not allowed");
    }
    return GetConnectString(Broker);
}

////////////////////////////////////////////////////////////////////////////////

CString ItemsForSale_t::GetDefaultSQL()
{
	return _T("[items_for_sale]");
}

////////////////////////////////////////////////////////////////////////////////

void ItemsForSale_t::DoFieldExchange(CFieldExchange* pFX)
{
	pFX->SetFieldType(CFieldExchange::outputColumn);
    if (CheckField(Field::SaleId))
    {
        RFX_Long(pFX, _T("[sale_id]"),       m_SaleId);
    }
    if (CheckField(Field::ItemId))
    {
        RFX_Long(pFX, _T("[item_id]"),       m_ItemId);
    }
    if (CheckField(Field::Price))
    {
        RFX_Long(pFX, _T("[item_price]"),    m_Price);
    }
    if (CheckField(Field::Quantity))
    {
        RFX_Long(pFX, _T("[quantity]"),      m_Quantity);
    }
    if (CheckField(Field::Commission))
    {
        RFX_Long(pFX, _T("[commission]"),    m_Commission);
    }
    if (CheckField(Field::MarketId))
    {
        RFX_Long(pFX, _T("[market_id]"),     m_MarketId);
    }
    if (CheckField(Field::SellerId))
    {
        RFX_Long(pFX, _T("[seller_id]"),     m_SellerId);
    }
    if (CheckField(Field::AddedDate))
    {
        RFX_Date(pFX, _T("[added_date]"),    m_AddedDate);
    }
    if (CheckField(Field::VerifiedDate))
    {
        RFX_Date(pFX, _T("[verified_date]"), m_VerifiedDate);
    }

    pFX->SetFieldType(CFieldExchange::param);
    if (CheckField(Field::ItemIdParam))
    {
        RFX_Long(pFX, _T("ItemIdParam"),     m_ItemIdParam);
    }
    if (CheckField(Field::SellerIdParam))
    {
        RFX_Long(pFX, _T("SellerIdParam"),   m_SellerIdParam);
    }
}

////////////////////////////////////////////////////////////////////////////////

SaleId_t
ItemsForSale_t::
AddItem(
    const wchar_t* pItemName,
          size_t   Price,
          size_t   Quantity,
    const wchar_t* pSellerName)
{
    if ((NULL == pItemName)   || (L'\0' == *pItemName)   ||
        (NULL == pSellerName) || (L'\0' == *pSellerName) ||
        (0 == Price))
    {
        LogError(L"Db::ItemsForSale_t::AddItem(): "
                 L"ItemName(%ls), Price(%d), SellerName(%ls)",
                 pItemName, Price, pSellerName);
        return 0;
    }
    using namespace Accounts::Db;
    SellerId_t SellerId = Sellers_t::AddSeller(pSellerName);
    if (0 == SellerId)
    {
        LogError(L"ItemsForSale_t::AddItem(): AddSeller(%ls) failed",
                 pSellerName);
        return 0;
    }
    ItemId_t ItemId = Items_t::AddItem(pItemName);
    if (0 == ItemId)
    {
        LogError(L"ItemsForSale_t::AddItem(): AddItem(%ls) failed",
                 pItemName);
        return 0;
    }
    return AddItem(ItemId, Price, Quantity, SellerId);
}

////////////////////////////////////////////////////////////////////////////////

SaleId_t
ItemsForSale_t::
AddItem(
    ItemId_t   ItemId,
    size_t     Price,
    size_t     Quantity,
    SellerId_t SellerId)
{
    SaleId_t ItemForSaleId = 0;
    try
    {
#if GLOBALDB
        CDatabase& db = GetDb(Broker);
#else
        CDatabase db;
        if (!db.OpenEx(Db::GetConnectString(Broker), CDatabase::noOdbcDialog))
        {
            LogError(L"DbItemsForSale_t::AddItem() failed in db.OpenEx()");
        }
        else
#endif
        {
//            ItemForSaleId = GetItemId(db, pItemName);
            if (0 == ItemForSaleId)
            {
                ItemForSaleId = AddItem(db, ItemId, Price, Quantity, SellerId);
            }
        }
    }
    catch (CDBException* e)
    {
        LogError(L"DbItemsForSale_t::AddItem(%d, %d, %d, %d) exception: %ls",
                 ItemId, Price, Quantity, SellerId, (LPCTSTR)e->m_strError);
        e->Delete();
    }
    return ItemForSaleId;
}

////////////////////////////////////////////////////////////////////////////////

SaleId_t
ItemsForSale_t::
AddItem(
    CDatabase& db,
    ItemId_t   ItemId,
    size_t     Price,
    size_t     Quantity,
    SellerId_t SellerId)
{
    ItemsForSale_t rs(&db, true);
    rs.m_strFilter = L"1 = 0";
//    rs.AddField(Field::AllFields);
    rs.AddField(Field::SaleId);
    rs.AddField(Field::ItemId);
    rs.AddField(Field::Price);
    rs.AddField(Field::Quantity);
    rs.AddField(Field::SellerId);
    rs.AddField(Field::Commission);
    rs.AddField(Field::MarketId);
    rs.AddField(Field::AddedDate);
    rs.AddField(Field::VerifiedDate);
    if (0 == rs.Open(CRecordset::dynaset, NULL, CRecordset::appendOnly))
    {
        LogError(L"DbItemForSale_t::AddItem(): failed at rs.Open()");
        return 0;
    }
    if (!rs.CanAppend())
    {
        LogError(L"DbItemForSale_t::AddItem(): failed at !rs.CanAppend()");
        return 0;
    }
    rs.AddNew();
//    rs.AddField(Field::AllFields);
    rs.m_ItemId     = ItemId;
    rs.m_Price      = long(Price);
    rs.m_Quantity   = long(Quantity);
    rs.m_SellerId   = SellerId;
    Accounts::Db::SetTimestamp(rs.m_AddedDate, CTime::GetCurrentTime());
    rs.SetFieldNull(&rs.m_Commission);
    rs.SetFieldNull(&rs.m_MarketId);
    rs.SetFieldNull(&rs.m_VerifiedDate);
    if (0 == rs.Update())
    {
        LogError(L"DbItemForSale_t::AddItem(): failed at rs.Update()");
        return 0;
    }

//    rs.ClearFields();
//    rs.AddField(Filed::SaleId);
    rs.m_strFilter = L"";
    if (0 == rs.Requery())
    {
        LogError(L"DbItemForSale_t::AddItem(): failed at rs.Requery()");
        return 0;
    }
    rs.SetAbsolutePosition(-1);
    LogAlways(L"Added ItemForSale to DB (%d: %d @ %d)", rs.m_SaleId, rs.m_Quantity, rs.m_Price);
    rs.Close();
    return rs.m_SaleId;
}

////////////////////////////////////////////////////////////////////////////////

size_t
ItemsForSale_t::
GetSaleCount(
    CDatabase& Db,
    ItemId_t   ItemId,
    SellerId_t SellerId)
{
    ItemsForSale_t rs(&Db);
//    static const wchar_t szSql[] = 
//        L"SELECT count(*) FROM items_for_sale WHERE item_id = ? AND seller_id = ?";
    static const wchar_t szSql[] = L"SELECT count(*) FROM [items_for_sale]";
//    rs.AddField(Field::SaleId);
    rs.m_strFilter = L"[item_id] = ? AND [seller_id] = ?";
    rs.m_ItemIdParam = ItemId;
    rs.AddParam(Field::ItemIdParam);
    rs.m_SellerIdParam = SellerId;
    rs.AddParam(Field::SellerIdParam);
    if (0 == rs.Open(CRecordset::forwardOnly, szSql, DefaultReadOnlyFlags))
    {
        LogError(L"ItemForSale_t::GetItemId(): failed at rs.Open()");
        return false;
    }
    CDBVariant vtCount;
    rs.GetFieldValue((short)0, vtCount, SQL_C_SLONG);
    return size_t(vtCount.m_lVal);
}

#if 0
ItemId_t
ItemForSale_t::
GetItemId(
    const wchar_t* pItemName)
{
    if ((NULL == pItemName) || (L'\0' == *pItemName))
    {
        throw std::invalid_argument("ItemForSale_t::GetItemId()");
    }

    ItemId_t ItemId = 0;
    CDatabase db;
    try
    {
#if 1
        CDatabase& db = GetDb(Broker);
#else
        if (!db.OpenEx(Db::GetConnectString(Broker), CDatabase::noOdbcDialog))
        {
            LogError(L"ItemsDb_t::AddItem() failed in db.OpenEx()");
        }
        else
#endif
        {
            ItemId = GetItemId(db, pItemName);
        }
    }
    catch (CDBException* e)
    {
        LogError(L"ItemForSale_t::AddItem() exception: %ls", e->m_strError);
        e->Delete();
    }
    return ItemId;
}

////////////////////////////////////////////////////////////////////////////////

ItemId_t
ItemForSale_t::
GetItemId(
          CDatabase& db,
    const wchar_t*   pItemName)
{
    ItemForSale_t rs(&db, pItemName);
    static const wchar_t szSql[] = 
        L"SELECT item_id FROM items WHERE item_name = ?";
    if (0 == rs.Open(CRecordset::forwardOnly, szSql, CRecordset::readOnly))
    {
        LogError(L"ItemForSale_t::GetItemId(): failed at rs.Open()");
        return false;
    }
    return rs.IsEOF() ? 0 : rs.m_item_id;
}
#endif

////////////////////////////////////////////////////////////////////////////////
// ItemsForSale_t diagnostics

#ifdef _DEBUG
void ItemsForSale_t::AssertValid() const
{
	CRecordset::AssertValid();
}

void ItemsForSale_t::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG

} // Db
