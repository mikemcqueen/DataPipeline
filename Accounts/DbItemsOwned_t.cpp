////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// DbItemsOwned_t.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DbItemsOwned_t.h"
#include "Log.h"
#include "ItemQuantityMap_t.h"

namespace Accounts
{
namespace Db
{

IMPLEMENT_DYNAMIC(ItemsOwned_t, CRecordset)

ItemsOwned_t::
ItemsOwned_t(
    CDatabase* pDb,
    bool       bAllowDefaultConnect)
:
    Recordset_t(pDb, bAllowDefaultConnect)
{
    m_item_id = 0;
    m_quantity = 0;
    m_character_id = 0;
    m_ItemIdParam = 0;
	m_nDefaultType = dynaset;
}

CString ItemsOwned_t::GetDefaultConnect()
{
    if (!AllowDefaultConnect())
    {
        throw std::logic_error("ItemsOwned_t::GetDefaultConnect() not allowed");
    }
    throw logic_error("ItemsOwned_t::GetDefaultConnect() not impelemented");
}

CString ItemsOwned_t::GetDefaultSQL()
{
	return _T("[items_owned]");
}

void ItemsOwned_t::DoFieldExchange(CFieldExchange* pFX)
{
    static const
    struct FieldData_t
    {
        Recordset_t::Field_t field;
        const wchar_t*       name;
        long ItemsOwned_t::* pData;
    } fields[] =
    {
        Field::ItemId,        L"[item_id]",       &ItemsOwned_t::m_item_id,
        Field::Quantity,      L"[quantity]",      &ItemsOwned_t::m_quantity,
        Field::CharacterId,   L"[character_id]",  &ItemsOwned_t::m_character_id
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
        RFX_Long(pFX, _T("CharacterIdParam"), m_CharacterIdParam);
    }
    if (CheckField(Field::ItemIdParam))
    {
        RFX_Long(pFX, _T("ItemIdParam"),      m_ItemIdParam);
    }
}

////////////////////////////////////////////////////////////////////////////////

void
ItemsOwned_t::
Load(
    ItemId_t           itemId,
    ItemQuantityMap_t& map)
{
    try
    {
        Load(GetDb(BuySell), itemId, map);
    }
    catch (CDBException* e)
    {
        LogError(L"ItemsOwned_t::Load() DbException: %s", (LPCTSTR)e->m_strError);
        e->Delete();
    }
}

////////////////////////////////////////////////////////////////////////////////

void
ItemsOwned_t::
Load(
    CDatabase&         db,
    ItemId_t           itemId,
    ItemQuantityMap_t& map)
{
    ItemsOwned_t rs(&db);
    rs.AddField(ItemsOwned_t::Field::ItemId);
    rs.AddField(ItemsOwned_t::Field::Quantity);
    rs.AddField(ItemsOwned_t::Field::CharacterId);
    rs.m_strFilter = L"[character_id] = ?";
    rs.m_CharacterIdParam = itemId;
    rs.AddParam(ItemsOwned_t::Field::CharacterIdParam);
    if (0 == rs.Open(CRecordset::forwardOnly, nullptr, Recordset_t::DefaultReadOnlyFlags))
    {
        throw logic_error("ItemsOwned_t::Load(): rs.Open() failed");
    }
    size_t totalItems = 0;
    for (; !rs.IsEOF(); rs.MoveNext())
    {
        auto [_, inserted] = map.insert(
            ItemQuantityMap_t::value_type(rs.m_item_id, rs.m_quantity));
        if (!inserted)
        {
            throw logic_error("ItemsOwned_t::Load(): map.insert() failed");
        }
        totalItems += rs.m_quantity;
    }
    LogAlways(L"ItemsOwned_t::Load Unique(%d) Total(%d)",
              map.size(), totalItems);
}

////////////////////////////////////////////////////////////////////////////////

size_t
ItemsOwned_t::
Write(
          CharacterId_t       characterId,
    const ItemQuantityMap_t& map)
{
    try
    {
        return Write(GetDb(BuySell), characterId, map);
    }
    catch (CDBException* e)
    {
        LogError(L"ItemsOwned_t::Write() DbException: %s", (LPCTSTR)e->m_strError);
        e->Delete();
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////

size_t
ItemsOwned_t::
Write(
          CDatabase&         db,
          CharacterId_t      characterId,
    const ItemQuantityMap_t& map)
{
    ItemsOwned_t rs(&db);
    rs.AddField(ItemsOwned_t::Field::ItemId);
    rs.AddField(ItemsOwned_t::Field::Quantity);
    rs.AddField(ItemsOwned_t::Field::CharacterId);
    if (0 == rs.Open(CRecordset::dynaset, nullptr, CRecordset::appendOnly))
    {
        LogError(L"ItemsOwned_t::Write(): failed at rs.Open()");
        return 0;
    }
    if (!rs.CanAppend())
    {
        LogError(L"ItemsOwned_t::Write(): failed at !rs.CanAppend()");
        return 0;
    }
    size_t total = 0;
    ItemQuantityMap_t::const_iterator it = map.begin();
    for (; map.end() != it; ++it)
    {
        rs.AddNew();
        rs.m_item_id  =     long(it->first);
        rs.m_quantity =     long(it->second);
        rs.m_character_id = long(characterId);
        if (0 == rs.Update())
        {
            LogError(L"ItemsOwned_t::Write(): failed at rs.Update()");
            break;
        }
        total += it->second;
    }
    LogAlways(L"ItemsOwned_t::Write() Unique(%d), Total(%d)",
              map.size(), total);
    return total;
}

////////////////////////////////////////////////////////////////////////////////

void
ItemsOwned_t::
AdjustQuantity(
    CharacterId_t characterId,
    ItemId_t      itemId,
    long          quantity)
{
    try
    {
        CDatabase& db = GetDb(BuySell);
        ItemsOwned_t rs(&db, false);
        rs.AddField(ItemsOwned_t::Field::ItemId);
        rs.AddField(ItemsOwned_t::Field::Quantity);
        rs.AddField(ItemsOwned_t::Field::CharacterId);
        rs.m_strFilter = L"[character_id] = ? AND [item_id] = ?";
        rs.m_CharacterIdParam = characterId;
        rs.AddParam(ItemsOwned_t::Field::CharacterIdParam);
        rs.m_ItemIdParam = itemId;
        rs.AddParam(ItemsOwned_t::Field::ItemIdParam);
        long remaining = quantity;
        bool updateFail = false;
        if (rs.Open(CRecordset::dynaset, nullptr, CRecordset::skipDeletedRecords))
        {
            if (!rs.IsEOF())
            {
                remaining += rs.m_quantity;
                if (0 < remaining)
                {
                    rs.Edit();
                    rs.m_quantity = remaining;
                    updateFail = !rs.Update();
                    if (!updateFail)
                    {
                        LogAlways(L"ItemsOwned_t::AdjustQuantity() Adjusted "
                                  L"Item(%d) Qty(%d) Remaining(%d)",
                                  itemId, quantity, remaining);
                    }
                }
                else
                {
                    rs.Delete();
                    LogAlways(L"ItemsOwned_t::AdjustQuantity() Deleted Item(%d)");
                }
                if (1 != rs.GetRecordCount())
                {
                    LogError(L"ItemsOwned_t::AdjustQuantity() Item(%d) has multiple records(%d)", 
                             itemId, rs.GetRecordCount());
                }
            }
            else if (0 < quantity)
            {
                if (rs.CanAppend())
                {
                    rs.AddNew();
                    rs.m_item_id = itemId;
                    rs.m_quantity = quantity;
                    rs.m_character_id = characterId;
                    updateFail = !rs.Update();
                    if (!updateFail)
                    {
                        LogAlways(L"ItemsOwned_t::AdjustQuantity() Added Item(%d)");
                    }
                }
            }
            if (0 > remaining)
            {
                LogError(L"ItemsOwned_t::AdjustQuantity() Item(%d) Qty(%d) Remaining(%d)", 
                         itemId, quantity, remaining);
            }
            if (updateFail)
            {
                LogError(L"ItemsOwned_t::AdjustQuantity() rs.Update() failed Item(%d) Quantity(%d)",
                         itemId, rs.m_quantity);
            }
        }
        else
        {
            LogError(L"ItemsOwned_t::AdjustQuantity(): failed at rs.Open()");
        }
    }
    catch (CDBException* e)
    {
        LogError(L"ItemsOwned_t::AdjustQuantity() DbException: %s", (LPCTSTR)e->m_strError);
        e->Delete();
    }
}

////////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
void ItemsOwned_t::AssertValid() const
{
	CRecordset::AssertValid();
}

void ItemsOwned_t::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG

////////////////////////////////////////////////////////////////////////////////

} // Db
} // Accounts

////////////////////////////////////////////////////////////////////////////////
