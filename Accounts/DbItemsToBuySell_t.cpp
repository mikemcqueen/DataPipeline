////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// ItemsToBuySell_t.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DbItemsToBuySell_t.h"
#include "AccountsDb.h"
#include "Log.h"

namespace Accounts
{
namespace Db
{

IMPLEMENT_DYNAMIC(ItemsToBuySell_t, CRecordset)

ItemsToBuySell_t::
ItemsToBuySell_t(
    CDatabase* pDb,
    bool       bAllowDefaultConnect)
:
    Recordset_t(pDb, bAllowDefaultConnect)
{
    m_ItemId = 0;
    m_LowBid = 0;
    m_HighBid = 0;
    m_BidIncrement = 0;
    m_LowAsk = 0;
    m_HighAsk = 0;
    m_AskIncrement = 0;
    m_ServerId = 0;
    m_CharacterId = 0;
	m_MaxToOwn = 0;
	m_MaxToSell = 0;
    m_MaxForSale = 0;

	m_nDefaultType = dynaset;
}

CString ItemsToBuySell_t::GetDefaultConnect()
{
    if (!AllowDefaultConnect())
    {
        throw std::logic_error("ItemsToBuySell_t::GetDefaultConnect() not allowed");
    }
    throw std::logic_error("ItemsToBuySell_t not impelemented");
}

CString ItemsToBuySell_t::GetDefaultSQL()
{
	return _T("[items_to_buysell]");
}

void ItemsToBuySell_t::DoFieldExchange(CFieldExchange* pFX)
{
    static const
    struct FieldData_t
    {
        Recordset_t::Field_t     field;
        const wchar_t*           name;
        long ItemsToBuySell_t::* pData;
    } fields[] =
    {
        Field::ItemId,        L"[item_id]",       &ItemsToBuySell_t::m_ItemId,
        Field::LowBid,        L"[low_bid]",       &ItemsToBuySell_t::m_LowBid,
        Field::HighBid,       L"[high_bid]",      &ItemsToBuySell_t::m_HighBid,
        Field::BidIncrement,  L"[bid_increment]", &ItemsToBuySell_t::m_BidIncrement,
        Field::LowAsk,        L"[low_ask]",       &ItemsToBuySell_t::m_LowAsk,
        Field::HighAsk,       L"[high_ask]",      &ItemsToBuySell_t::m_HighAsk,
        Field::AskIncrement,  L"[ask_increment]", &ItemsToBuySell_t::m_AskIncrement,
        Field::MaxToOwn,      L"[max_to_own]",    &ItemsToBuySell_t::m_MaxToOwn,
        Field::MaxToSell,     L"[max_to_sell]",   &ItemsToBuySell_t::m_MaxToSell,
        Field::MaxForSale,    L"[max_for_sale]",  &ItemsToBuySell_t::m_MaxForSale,
        Field::ServerId,      L"[server_id]",     &ItemsToBuySell_t::m_ServerId,
        Field::CharacterId,   L"[character_id]",  &ItemsToBuySell_t::m_CharacterId
    };

	pFX->SetFieldType(CFieldExchange::outputColumn);
    for (int index = 0; index < _countof(fields); ++index)
    {
        if (CheckField(fields[index].field))
        {
            RFX_Long(pFX, fields[index].name, this->*(fields[index].pData));
        }
    }
    pFX->SetFieldType(CFieldExchange::param);
    if (CheckField(Field::CharacterIdParam))
    {
        RFX_Long(pFX, _T("CharacterIdParam"),     m_CharacterIdParam);
    }
    if (CheckField(Field::ServerIdParam))
    {
        RFX_Long(pFX, _T("ServerIdParam"),     m_ServerIdParam);
    }
}

////////////////////////////////////////////////////////////////////////////////

void
ItemsToBuySell_t::
Load(
    CharacterId_t     id,
    ItemBuySellMap_t& map)
{
    try
    {
        using namespace Accounts::Db;
        Load(GetDb(BuySell), id, map);
    }
    catch (CDBException* e)
    {
        LogError(L"ItemsToBuySell_t::Load() DbException: %s", (LPCTSTR)e->m_strError);
        e->Delete();
    }
}

////////////////////////////////////////////////////////////////////////////////

void
ItemsToBuySell_t::
Load(
    CDatabase&        db,
    CharacterId_t     id,
    ItemBuySellMap_t& map)
{
    ItemsToBuySell_t rs(&db);
    rs.AddField(ItemsToBuySell_t::Field::ItemId);
    rs.AddField(ItemsToBuySell_t::Field::LowBid);
    rs.AddField(ItemsToBuySell_t::Field::HighBid);
    rs.AddField(ItemsToBuySell_t::Field::BidIncrement);
    rs.AddField(ItemsToBuySell_t::Field::LowAsk);
    rs.AddField(ItemsToBuySell_t::Field::HighAsk);
    rs.AddField(ItemsToBuySell_t::Field::AskIncrement);
    rs.AddField(ItemsToBuySell_t::Field::MaxToOwn);
    rs.AddField(ItemsToBuySell_t::Field::MaxToSell);
    rs.AddField(ItemsToBuySell_t::Field::MaxForSale);
//    rs.AddField(ItemsToBuySell_t::Field::ServerId);
//    rs.AddField(ItemsToBuySell_t::Field::CharacterId);

    rs.m_strFilter = L"([character_id] = 0 OR [character_id] = ?)";
    rs.m_CharacterIdParam = id;
    rs.AddParam(ItemsToBuySell_t::Field::CharacterIdParam);

    if (0 == rs.Open(CRecordset::forwardOnly, nullptr, Recordset_t::DefaultReadOnlyFlags))
    {
        throw std::logic_error("ItemsToBuySell_t::Load(): rs.Open() failed");
    }
    for (; !rs.IsEOF(); rs.MoveNext())
    {
        BuySellData_t buySellData(rs);
        auto [_, inserted] = map.insert(
            ItemBuySellMap_t::value_type(rs.m_ItemId, buySellData));
        if (!inserted)
        {
            LogError(L"ItemsToBuySell_t::Load(): map.insert(%d) failed", rs.m_ItemId);
            throw std::logic_error("ItemsToBuySell_t::Load(): map.insert() failed");
        }
    }
    LogAlways(L"ItemsToBuySell_t::Load() map.size(%d)", map.size());
}

////////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
void ItemsToBuySell_t::AssertValid() const
{
	CRecordset::AssertValid();
}

void ItemsToBuySell_t::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG

////////////////////////////////////////////////////////////////////////////////

} // Db
} // Accounts

////////////////////////////////////////////////////////////////////////////////
