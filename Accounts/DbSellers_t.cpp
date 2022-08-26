// Sellers_t.h : Implementation of the Sellers_t class

#include "stdafx.h"
#include "DbSellers_t.h"
#include "Log.h"
#include "AccountsDb.h"

namespace Accounts
{
namespace Db
{

IMPLEMENT_DYNAMIC(Sellers_t, CRecordset)

Sellers_t::
Sellers_t(
    CDatabase* pdb)
:
    Recordset_t(pdb, false)
{
    Init();
}

Sellers_t::
Sellers_t(
    CDatabase* pdb,
    const wchar_t* pParam)
:
    Recordset_t(pdb, false)
{
    Init();
//    m_nParams = 1;
    m_strParam1 = pParam;
}

void
Sellers_t::Init()
{
	m_seller_id = 0;
	m_seller_name = L"";
	m_market_id = 0;
    SecureZeroMemory(&m_added_date, sizeof(m_added_date));
    SecureZeroMemory(&m_verified_date, sizeof(m_verified_date));
//	m_nFields = 5;
	m_nDefaultType = dynaset;
}

CString Sellers_t::GetDefaultConnect()
{
	return GetConnectString(Sellers);
}

CString Sellers_t::GetDefaultSQL()
{
	return _T("[sellers]");
}

void Sellers_t::DoFieldExchange(CFieldExchange* pFX)
{
    //  Todo: RegisterField(pColumnText, Type& Member, FieldFlag); 

	pFX->SetFieldType(CFieldExchange::outputColumn);
    if (CheckField(Field::SellerId))
    {
        RFX_Long(pFX, L"[seller_id]",     m_seller_id);
    }
    if (CheckField(Field::SellerName))
    {
        RFX_Text(pFX, L"[seller_name]",   m_seller_name);
    }
    if (CheckField(Field::MarketId))
    {
        RFX_Long(pFX, L"[market_id]",     m_market_id);
    }
    if (CheckField(Field::AddedDate))
    {
        RFX_Date(pFX, L"[added_date]",    m_added_date);
    }
    if (CheckField(Field::VerifiedDate))
    {
        RFX_Date(pFX, L"[verified_date]", m_verified_date);
    }

    pFX->SetFieldType(CFieldExchange::param);
    if (CheckField(Field::StringParam1))
    {
        RFX_Text(pFX, _T("param1"), m_strParam1);
    }
}

////////////////////////////////////////////////////////////////////////////////

SellerId_t
Sellers_t::
GetSellerId(
    const wchar_t* pSellerName)
{
    if ((NULL == pSellerName) || (L'\0' == *pSellerName))
    {
        throw std::invalid_argument("Sellers_t::GetSellerId()");
    }

    SellerId_t SellerId = 0;
    //CDatabase db;
    try
    {
#if 1 || GLOBALDB
        CDatabase& db = GetDb(Sellers);
#else
        CDatabase db;
        if (!db.OpenEx(Db::GetConnectString(Sellers), CDatabase::noOdbcDialog))
        {
            LogError(L"DbSellers_t::AddSeller() failed in db.OpenEx()");
        }
        else
#endif
        {
            SellerId = GetSellerId(db, pSellerName);
        }
    }
    catch (CDBException* e)
    {
        LogError(L"DbSellers_t::AddSeller() exception: %ls", (LPCTSTR)e->m_strError);
        e->Delete();
    }
    return SellerId;
}

////////////////////////////////////////////////////////////////////////////////

SellerId_t
Sellers_t::
GetSellerId(
          CDatabase& db,
    const wchar_t*   pSellerName)
{
    Sellers_t rs(&db, pSellerName);
    rs.AddField(Field::SellerId);
    rs.m_strFilter = L"[seller_name] = ?";
    rs.AddParam(Field::StringParam1);
    if (0 == rs.Open(CRecordset::forwardOnly, NULL, DefaultReadOnlyFlags))
    {
        LogError(L"Sellers_t::GetSellerId(): failed at rs.Open()");
        return false;
    }
    return rs.IsEOF() ? 0 : rs.m_seller_id;
}

////////////////////////////////////////////////////////////////////////////////

SellerId_t
Sellers_t::
AddSeller(
    const wchar_t* pSellerName)
{
    SellerId_t SellerId = 0;
    //CDatabase db;
    try
    {
#if 1 || GLOBALDB
        CDatabase& db = GetDb(Sellers);
#else
        CDatabase db;
        if (!db.OpenEx(Db::GetConnectString(Sellers), CDatabase::noOdbcDialog))
        {
            LogError(L"DbSellers_t::AddSeller() failed in db.OpenEx()");
        }
        else
#endif
        {
            SellerId = GetSellerId(db, pSellerName);
            if (0 == SellerId)
            {
                SellerId = AddSeller(db, pSellerName);
            }
        }
    }
    catch (CDBException* e)
    {
        LogError(L"DbSellers_t::AddSeller(%ls) exception: %ls",
                 pSellerName, (LPCTSTR)e->m_strError);
        e->Delete();
    }
    return SellerId;
}

////////////////////////////////////////////////////////////////////////////////

SellerId_t
Sellers_t::
AddSeller(
          CDatabase& db,
    const wchar_t*   pSellerName)
{
    Sellers_t rs(&db);
    rs.AddField(Field::SellerId);
    rs.AddField(Field::SellerName);
    rs.AddField(Field::MarketId);
    rs.AddField(Field::AddedDate);
    rs.AddField(Field::VerifiedDate);
    rs.m_strFilter = L"1 = 0";
    if (0 == rs.Open(CRecordset::dynaset, NULL, CRecordset::appendOnly))
    {
        LogError(L"DbSellers_t::AddSeller(): failed at rs.Open()");
        return 0;
    }
    if (!rs.CanAppend())
    {
        LogError(L"DbSellers_t::AddSeller(): failed at !rs.CanAppend()");
        return 0;
    }
    rs.AddNew();
    rs.m_seller_name = pSellerName;
    SetTimestamp(rs.m_added_date, CTime::GetCurrentTime());
    rs.SetFieldNull(&rs.m_market_id);
    rs.SetFieldNull(&rs.m_verified_date);
    if (0 == rs.Update())
    {
        LogError(L"DbSellers_t::AddSeller(): failed at rs.Update()");
        return 0;
    }
    rs.m_strFilter = L"";
    if (0 == rs.Requery())
    {
        LogError(L"DbSellers_t::AddSeller(): failed at rs.Requery()");
        return 0;
    }
    rs.SetAbsolutePosition(-1);
    LogAlways(L"DBSellers_t::AddSeller(): Added '%ls' (%d)", (LPCTSTR)rs.m_seller_name, rs.m_seller_id);
    rs.Close();
    return rs.m_seller_id;
}

/////////////////////////////////////////////////////////////////////////////
// Sellers_t diagnostics

#ifdef _DEBUG
void Sellers_t::AssertValid() const
{
	CRecordset::AssertValid();
}

void Sellers_t::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG

} // Db
} // Accounts