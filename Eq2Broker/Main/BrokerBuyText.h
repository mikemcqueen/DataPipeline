///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// BrokerBuyText.h
//
///////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_BROKERBUYTEXT_H
#define Include_BROKERBUYTEXT_H

#include "BrokerBuyTypes.h"
#include "Price_t.h"

namespace Broker
{
namespace Buy
{

typedef TextTableData_t<Table::RowCount, Table::CharsPerRow, Table::ColumnCount> TextBase_t;

///////////////////////////////////////////////////////////////////////////////

class Text_t :
    public TextBase_t
{
public:

    Text_t() = default;

    explicit
    Text_t(
        const TextBase_t& textBase)
        :
        TextBase_t(textBase)
    { }

    Text_t(
        const size_t* pColumnWidths,
              size_t  ColumnCount)
        :
        TextBase_t(pColumnWidths, ColumnCount)
    { }

    long
    GetQuantity(
        size_t Row) const
    {
        return GetQuantity(GetRow(Row));
    }

    long
    GetQuantity(
        const wchar_t* pRow) const
    {
        const wchar_t* pQuantityText = &pRow[GetColumnOffset(Table::QuantityColumn)];
        size_t Quantity = _wtol(pQuantityText);
        if (0 == Quantity)
        {
            if (L'0' != pQuantityText[0])
            {
                Quantity = 1;
            }
        }
        return Quantity;
    }

    const wchar_t* 
    GetItemName(
        size_t Row) const
    {
        return GetItemName(GetRow(Row));
    }

    const wchar_t* 
    GetItemName(
        const wchar_t* pRow) const
    {
        return &pRow[GetColumnOffset(Table::ItemNameColumn)];
    }

    void
    GetSellerName(
        size_t        Row,
        std::wstring& strSellerName) const
    {
        return GetSellerName(GetRow(Row), strSellerName);
    }

    void
    GetSellerName(
        const wchar_t* pRow,
        std::wstring&  strSellerName) const
    {
        const wchar_t* pSellerText = &pRow[GetColumnOffset(Table::SellerNameColumn)];
        static const wchar_t szVisit[]   = L"Visit ";
        static const size_t  VisitLength = 6;

        const wchar_t* pEndName = nullptr;
        size_t Offset = 0;
        if (0 == wcsncmp(pSellerText, szVisit, VisitLength))
        {
            pSellerText += VisitLength;
            pEndName = wcschr(pSellerText + Offset, L' ');
        }
        if (nullptr != pEndName)
        {
            strSellerName.assign(pSellerText, pEndName - pSellerText);
        }
        else
        {
            strSellerName.assign(pSellerText);
        }
    }

    long
    GetPrice(
        size_t row) const
    {
        return GetPrice(GetRow(row));
    }

    long
    GetPrice(
        const wchar_t* pRow) const
    {
        return GetPrice(pRow, GetColumnOffset(Table::PriceColumn));
    }

    static
    long
    GetPrice(
        const wchar_t* pRow,
              size_t   priceOffset)
    {
        Price_t Price;
        if (Price.Parse(&pRow[priceOffset])) {
            return Price.GetPrice();
        }
        return 0;
    }
};

///////////////////////////////////////////////////////////////////////////////

} // Buy
} // Broker

#endif // Include_BROKERBUYTEXT_H

///////////////////////////////////////////////////////////////////////////////
