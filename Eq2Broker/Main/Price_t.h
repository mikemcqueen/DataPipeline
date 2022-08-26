/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// Price_t.h
//
// Price_t class.
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_PRICE_T_H
#define Include_PRICE_T_H

/////////////////////////////////////////////////////////////////////////////

class Price_t
{

public:

    struct Data_t
    {
        Data_t() :
            Price(0)
        { }

        size_t Price;
    };

private:

    Data_t m_Data;

public:

    Price_t()
    {}

    Price_t(
        const Data_t& Data)
    :
        m_Data(Data)
    { }

    bool
    Parse(
        const wchar_t* pText,
        bool           bAllowSmallCoin = true);

    // Accessors:
    size_t GetPrice() const       { return m_Data.Price; }

    const Data_t& GetData() const { return m_Data; }

//    bool IsValid() const          { return (0 != GetPage()) && (0 != GetLastPage()); }

    void Reset()                  { m_Data.Price = 0; }

};

/////////////////////////////////////////////////////////////////////////////

#endif Include_PRICE_T_H

/////////////////////////////////////////////////////////////////////////////
