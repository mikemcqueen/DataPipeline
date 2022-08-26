/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// PageNumber_t.h
//
// PageNumber_t class.
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_PAGENUMBER_T_H
#define Include_PAGENUMBER_T_H

/////////////////////////////////////////////////////////////////////////////

class PageNumber_t
{

public:

    struct Data_t
    {
        size_t Page;
        size_t LastPage;

        Data_t() { Clear(); }
        void Clear() { Page = 0; LastPage = 1; }
    };

private:

    Data_t m_Data;

public:

    PageNumber_t() {}
    PageNumber_t(const Data_t& Data) : m_Data(Data) {}

    bool
    Parse(
        const wchar_t* pText);

    // Accessors:
    size_t GetPage() const        { return m_Data.Page; }
    size_t GetLastPage() const    { return m_Data.LastPage; }

    const Data_t& GetData() const { return m_Data; }

    bool IsValid() const          { return 0 != GetPage(); }

    void Reset()                  { m_Data.Clear(); }
};

/////////////////////////////////////////////////////////////////////////////

#endif Include_PAGENUMBER_T_H

/////////////////////////////////////////////////////////////////////////////
