////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2009 Mike McQueen.  All rights reserved.
//
// ItemQuantityMap_t.h
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Log.h"
#include "GameId.h"
#include "DbItems_t.h"

////////////////////////////////////////////////////////////////////////////////

class ItemQuantityMap_t :
  public std::map<ItemId_t, size_t>
{
    std::wstring m_mapName;

public:

    ItemQuantityMap_t(
         const wchar_t* mapName = nullptr) :
         m_mapName((mapName != nullptr) ? mapName : L"ItemQuantityMap_t")
    {
    }

    size_t
    GetQuantity(
        ItemId_t itemId) const
    {
        const_iterator it = find(itemId);
        return (end() != it) ? it->second : 0;
    }

    void
    AdjustQuantity(
        ItemId_t itemId,
        long     quantity,
        bool     showUnderflowError = true)
    {
        long remaining = quantity;
        iterator itOwned = find(itemId);
        if (end() != itOwned)
        {
            remaining += long(itOwned->second);
            if (0 < remaining)
            {
                itOwned->second = remaining;
                LogAlways(L"%s::AdjustQuantity() Adjusted Item(%d) Qty(%d)",
                          m_mapName.c_str(), itemId, remaining);
            }
            else
            {
                erase(itOwned);
                LogAlways(L"%s::AdjustQuantity() Removed Item(%d)",
                          m_mapName.c_str(), itemId);
            }
        }
        else if (0 < quantity)
        {
            if (insert(std::make_pair(itemId, quantity)).second)
            {
                LogAlways(L"%s::AdjustQuantity() Added Item(%d) Qty(%d)",
                          m_mapName.c_str(), itemId, quantity);
            }
            else
            {
                LogError(L"%s::AdjustQuantity() insert() ItemId(%d) Quantity(%d) failed",
                         m_mapName.c_str(), itemId, quantity);
            }
        }
        if ((0 > remaining) && showUnderflowError)
        {
            LogError(L"%s::AdjustQuantity() Underflow Item(%d) Qty(%d) Remaining(%d)", 
                     m_mapName.c_str(), itemId, quantity, remaining);
        }
    }

    void
    Dump() const
    {
        size_t total = 0;
        LogAlways(L"%s:", m_mapName.c_str());
        for (const_iterator it = begin(); end() != it; ++it)
        {
            using namespace Accounts::Db;
            LogAlways(L"  %3d x (%5d) (%s)", it->second, it->first,
                      Items_t::GetItemName(it->first));
            total += it->second;
        }
        LogAlways(L"%s: Unique(%d) Total(%d)", m_mapName.c_str(), 
                  size(), total);
    }
};
