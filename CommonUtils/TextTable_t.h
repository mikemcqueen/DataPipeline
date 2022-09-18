/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// TextTable_t.h
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#include "Log.h"

#ifndef Include_TEXTTABLE_T_H
#define Include_TEXTTABLE_T_H

/////////////////////////////////////////////////////////////////////////////

class TextTable_i
{

public:

    virtual size_t           GetRowCount() const        = 0;
    virtual wchar_t*         GetRow(size_t Row)         = 0;
    virtual size_t           GetRowWidth() const        = 0;

    virtual size_t           GetColumnCount() const     = 0;
    virtual const size_t*    GetColumnWidths() const    = 0;

    virtual void             SetEndRow(size_t Row)      = 0;

    size_t
    GetColumnWidth(
        size_t Column) const
    {
        const size_t Count = GetColumnCount();
        if (Count <= Column)
            throw std::invalid_argument("TextTable_i::GetColumnWidth()");
        return GetColumnWidths()[Column];
    }

    size_t
    GetColumnOffset(
        size_t Column) const
    {
        const size_t Count = GetColumnCount();
        if (Count <= Column)
            throw std::invalid_argument("TextTable_i::GetColumnOffset()");
        const size_t* pColumnWidths = GetColumnWidths();
        size_t Offset = 0;
        for (size_t c = 0; c < Column; ++c)
        {
            Offset += pColumnWidths[c];
        }
        return Offset;
    }
};

/////////////////////////////////////////////////////////////////////////////

template<
    size_t RowCount,
    size_t CharsPerRow,
    size_t MaxColumnCount>
class TextTableData_t
{
private:

    typedef TextTableData_t<RowCount, CharsPerRow, MaxColumnCount> This_t;

public:

    typedef wchar_t Row_t[CharsPerRow];
    static const wchar_t Marker     = L'\1';

private:

    Row_t   m_Buffer[RowCount];
    size_t  m_ColumnWidths[MaxColumnCount];
    size_t  m_ColumnCount;
    size_t  m_EndRow;

public:

    // allowed, for threadqueue_t, should init everything to zero
    TextTableData_t()
    {
        SecureZeroMemory(this, sizeof(This_t));
    }

    TextTableData_t(
        const size_t* pColumnWidths,
              size_t  ColumnCount)
    :
        m_ColumnCount(ColumnCount)
    {
        SetColumnWidths(pColumnWidths, ColumnCount);
    }

    TextTableData_t(const This_t& rhs)
    {
        *this = rhs;
    }

    This_t& operator=(const This_t& rhs)
    {
        if (this != &rhs)
        {
            memcpy(this, &rhs, sizeof(This_t));
        }
        return *this;
    }


    void Clear()
    {
        SecureZeroMemory(&m_Buffer, sizeof(m_Buffer));
        m_EndRow = 0;
    }

    bool IsEmpty() const
    {
        return 0 == GetEndRow();
    }

    size_t GetRowCount() const    { return RowCount; }
    size_t GetCharsPerRow() const { return CharsPerRow; }

#if 1
    const wchar_t*
    GetRow(
        size_t Row) const
    {
        return const_cast<This_t*>(this)->GetBuffer(Row);
    }
#else
    const Row_t&
    GetRow(
        size_t Row) const
    {
        return reinterpret_cast<Row_t&>(*const_cast<This_t*>(this)->GetBuffer(Row));
    }
#endif

    wchar_t*
    GetBuffer(
        size_t Row)
    {
        if (Row < RowCount)
            return m_Buffer[Row];
        throw std::invalid_argument("TextTableData_t::GetBuffer()");
    }

    size_t
    GetColumnCount() const  { return m_ColumnCount; }

    size_t
    GetColumnWidth(
        size_t Column) const
    {
        if (m_ColumnCount > Column)
            return m_ColumnWidths[Column];
        throw std::invalid_argument("TextTableData_t::GetColumnWidth()");
    }

    size_t
    GetColumnOffset(
        size_t Column) const
    {
        if (m_ColumnCount > Column)
        {
            size_t Offset = 0;
            for (size_t cw = 0; cw < Column; ++cw)
            {
                Offset += m_ColumnWidths[cw];
            }
            return Offset;
        }
        throw std::invalid_argument("TextTableData_t::GetColumnOffset()");
    }

    void
    SetColumnWidths(
        const size_t* pColumnWidths,
        size_t        ColumnCount)
    {
        if (MaxColumnCount < ColumnCount)
            throw std::invalid_argument("TextTableData_t::SetColumnWidths()");
        Clear();
        memcpy(&m_ColumnWidths, pColumnWidths, ColumnCount * sizeof(m_ColumnWidths[0]));
        m_ColumnCount = ColumnCount;
    }

    void
    SetEndRow(
        size_t Row)
    {
        if (RowCount < Row)
            throw std::invalid_argument("TextTableData_t::SetEndRow()");
        m_EndRow = Row;
    }

    size_t
    GetEndRow() const
    {
        return m_EndRow;
    }

    const size_t *
    GetColumnWidths() const
    {
        return &m_ColumnWidths[0];
    }

    bool
    IsMarkedRow(
              size_t   Row,
        const wchar_t* pszTag = nullptr) const
    {
        const wchar_t* pszRow = GetRow(Row);
        if (Marker == pszRow[0])
        {
            if (nullptr != pszTag)
            {
                return 0 == wcscmp(&pszRow[1], pszTag);
            }
            return true;
        }
        return false;
    }

    bool
    CompareRow(
        size_t        iMyRow,
        const This_t& text,
        size_t        iTheRow) const
    {
        //
        // TODO: just memcmp?
        //

        bool bMatch = true;
        const wchar_t* pMyRow  = GetRow(iMyRow);
        const wchar_t* pTheRow = text.GetRow(iTheRow);

        size_t pos = 0;
        size_t iColumn = 0;
        for (; iColumn < text.GetColumnCount(); ++iColumn)
        {
            if (0 != wcscmp(&pMyRow[pos], &pTheRow[pos]))
            {
                bMatch = false;
                break;
            }
            pos += text.GetColumnWidth(iColumn);
        }
        return bMatch;
    }

    void
    Dump(
        const wchar_t* pszText = nullptr,
              bool     bAlways = false) const
    {
        wchar_t* pszTag = nullptr;
        if (nullptr != pszText)
            LogInfo(pszText);
        for (size_t Row = 0; Row < RowCount; ++Row)
        {
            DumpRow(Row, pszTag, bAlways);
        }
    }

    void
    DumpRow(
        size_t Row,
        const wchar_t* pszTag = nullptr,
        bool   bAlways = false) const
    {
        ASSERT(Row < RowCount);

        wchar_t buf[CharsPerRow * 2]; // probably big enough, not exact
        if (nullptr != pszTag)
            wsprintf(buf, L"%ls: %02d: ", pszTag, Row);
        else
            wsprintf(buf, L"%02d: ", Row);

        const wchar_t* pszRow = GetRow(Row);
        if (IsMarkedRow(Row))
        {
            wcscat_s(buf, _countof(buf), &pszRow[1]);
        }
        else
        {
            size_t pos = 0;
            for (size_t iColumn = 0; iColumn < m_ColumnCount; ++iColumn)
            {
                if (iColumn > 0)
                    wcscat_s(buf, _countof(buf), L",");
            
                wchar_t minibuf[256];
                if (0 < m_ColumnWidths[iColumn])
                {
                    wsprintf(minibuf, L"\"%ls\"", &pszRow[pos]);
                    wcscat_s(buf, _countof(buf), minibuf);
                }
                else
                {
                    wcscat_s(buf, L"[empty]");
                }
                pos += GetColumnWidth(iColumn);
            }
        }
        if (bAlways)
            LogAlways(buf);
        else
            LogInfo(buf);
    }
};

/////////////////////////////////////////////////////////////////////////////
//
// TextTable_t
//
/////////////////////////////////////////////////////////////////////////////

template<
    size_t RowCount,
    size_t CharsPerRow,
    size_t ColumnCount>
class TextTable_t :
    public TextTable_i
{
public:

    typedef TextTable_t<RowCount, CharsPerRow, ColumnCount> This_t;

    typedef TextTableData_t<RowCount, CharsPerRow, ColumnCount> Data_t;

private:

    Data_t m_Data;

public:

    TextTable_t(
        const size_t* pColumnWidths,
        size_t        MaxColumnCount)
    :
        m_Data(pColumnWidths, MaxColumnCount)
    {
    }

    This_t& operator=(const Data_t& Data)
    {
        m_Data = Data;
		return *this; // NOTE added to get to compile
    }

    //
    // TextTable_i virtual:
    //

    virtual size_t           GetRowCount() const     { return m_Data.GetRowCount(); }
    virtual wchar_t*         GetRow(size_t Row)      { return m_Data.GetBuffer(Row); }
    virtual size_t           GetRowWidth() const     { return m_Data.GetCharsPerRow(); }

    virtual size_t           GetColumnCount() const  { return ColumnCount; }
    virtual const size_t*    GetColumnWidths() const { return m_Data.GetColumnWidths(); }

    virtual void             SetEndRow(size_t Row)
    {
        m_Data.SetEndRow(Row);
#if 1
        if (Row < RowCount)
            MarkRow(Row, L"END");
        for (++Row; Row < RowCount; ++Row)
            MarkRow(Row, L"UNUSED");
#endif
    }

    const wchar_t*
    GetRow(
        size_t Row) const
    {
        return m_Data.GetRow(Row);
    }

    size_t
    GetEndRow() const
    {
        return m_Data.GetEndRow();
    }

    bool
    IsEmpty() const
    {
        return m_Data.IsEmpty();
    }

    const Data_t&
    GetData() const
    {
        return m_Data;
    }

    size_t
    GetColumnOffset(
        size_t Column) const
    {
        return m_Data.GetColumnOffset(Column);
    }

    bool
    CompareRow(
        size_t        iMyRow,
        const This_t& text,
        size_t        iTheRow) const
    {
        return m_Data.CompareRow(iMyRow, text.GetData(), iTheRow);
    }

    void
    SetColumnWidths(
        const size_t* pColumnWidths,
        size_t        ColumnCount)
    {
        m_Data.SetColumnWidths(pColumnWidths, ColumnCount);
    }

    void
    Dump(
        const wchar_t* pszText = nullptr,
              bool     bAlways = false) const
    {
        m_Data.Dump(pszText, bAlways);
    }

    void
    DumpRow(
        size_t Row,
        const wchar_t* pszTag = nullptr) const
    {
        m_Data.DumpRow(Row, pszTag);
    }

// NOTE made this public to get it compiling
    void
    Clear()
    {
        m_Data.Clear();
    }

private:


    size_t
    GetColumnWidth(
        size_t Column) const
    {
        return m_Data.GetColumnWidth(Column);
    }

    bool
    IsEndRow(
        size_t Row) const
    {
        return IsMarkedRow(Row, L"END");
    }

    bool
    IsBadRow(
        size_t Row) const
    {
        // TODO: hacko until TextTable_t becomes DCR::TextTable_t,
        // and DCR::ReadTable() takes it as a parameter.
        const wchar_t* pszRow = GetRow(Row);
        return 0 == wcscmp(pszRow, L"BAD");
    }

protected:

    bool
    IsMarkedRow(
        size_t         Row,
        const wchar_t* pszTag = nullptr) const
    {
        return m_Data.IsMarkedRow(Row, pszTag);
    }

    void
    MarkRow(
        size_t Row,
        wchar_t* pszTag)
    {
//Data.MarkRow(Row, pszTag);
        wchar_t* pszRow = (wchar_t*)m_Data.GetRow(Row);
        pszRow[0] = Data_t::Marker;
        wcscpy_s(&pszRow[1], CharsPerRow - 1, pszTag);
    }

private:

    // Explicitly disabled:
    TextTable_t();
    TextTable_t(const TextTable_t&);
    TextTable_t& operator=(const TextTable_t&);
};

/////////////////////////////////////////////////////////////////////////////
//
// TextTable2_t
//
/////////////////////////////////////////////////////////////////////////////

template<class Data_t>
class TextTable2 :
    public TextTable_i
{
public:

    typedef TextTable2<Data_t> This_t;

private:

    Data_t m_Data;

public:

    TextTable2(
        const size_t* pColumnWidths,
        size_t        MaxColumnCount)
    :
        m_Data(pColumnWidths, MaxColumnCount)
    {
    }

/*
    This_t&
    operator=(const Data_t& Data)
    {
        m_Data = Data;
    }
*/

    //
    // TextTable_i virtual:
    //

    virtual size_t           GetRowCount() const     { return m_Data.GetRowCount(); }
    virtual wchar_t*         GetRow(size_t Row)      { return m_Data.GetBuffer(Row); }
    virtual size_t           GetRowWidth() const     { return m_Data.GetCharsPerRow(); }

    virtual size_t           GetColumnCount() const  { return m_Data.GetColumnCount(); }
    virtual const size_t*    GetColumnWidths() const { return m_Data.GetColumnWidths(); }

    virtual void             SetEndRow(size_t Row)
    {
        m_Data.SetEndRow(Row);
#if 1
        if (Row < GetRowCount())
            MarkRow(Row, L"END");
        for (++Row; Row < GetRowCount(); ++Row)
            MarkRow(Row, L"UNUSED");
#endif
    }

    const wchar_t*
    GetRow(
        size_t Row) const
    {
        return m_Data.GetRow(Row);
    }

    size_t
    GetEndRow() const
    {
        return m_Data.GetEndRow();
    }

    bool
    IsEmpty() const
    {
        return m_Data.IsEmpty();
    }

    const Data_t&
    GetData() const
    {
        return m_Data;
    }

    size_t
    GetColumnOffset(
        size_t Column) const
    {
        return m_Data.GetColumnOffset(Column);
    }

    bool
    CompareRow(
        size_t        iMyRow,
        const This_t& text,
        size_t        iTheRow) const
    {
        return m_Data.CompareRow(iMyRow, text, iTheRow);
    }

    void
    SetColumnWidths(
        const size_t* pColumnWidths,
        size_t        ColumnCount)
    {
        m_Data.SetColumnWidths(pColumnWidths, ColumnCount);
    }

    void
    Dump(
        const wchar_t* pszText = nullptr,
              bool     bAlways = false) const
    {
        m_Data.Dump(pszText, bAlways);
    }

    void
    DumpRow(
        size_t Row,
        const wchar_t* pszTag = nullptr) const
    {
        m_Data.DumpRow(Row, pszTag);
    }

private:

    void
    Clear()
    {
        m_Data.Clear();
    }

    size_t
    GetColumnWidth(
        size_t Column) const
    {
        return m_Data.GetColumnWidth(Column);
    }

    bool
    IsEndRow(
        size_t Row) const
    {
        return IsMarkedRow(Row, L"END");
    }

    bool
    IsBadRow(
        size_t Row) const
    {
        // TODO: hacko until TextTable_t becomes DCR::TextTable_t,
        // and DCR::ReadTable() takes it as a parameter.
        const wchar_t* pszRow = GetRow(Row);
        return 0 == wcscmp(pszRow, L"BAD");
    }

protected:

    bool
    IsMarkedRow(
        size_t         Row,
        const wchar_t* pszTag = nullptr) const
    {
        return m_Data.IsMarkedRow(Row, pszTag);
    }

    void
    MarkRow(
        size_t Row,
        const wchar_t* pszTag)
    {
//Data.MarkRow(Row, pszTag);
        wchar_t* pszRow = (wchar_t*)m_Data.GetRow(Row);
        pszRow[0] = Data_t::Marker;
        wcscpy_s(&pszRow[1], GetRowWidth() - 1, pszTag);
    }

private:

    // Explicitly disabled:
    TextTable2();
    TextTable2(const TextTable2&);
    TextTable2& operator=(const TextTable2&);
};

/////////////////////////////////////////////////////////////////////////////

#endif // Include_TEXTTABLE_T_H

/////////////////////////////////////////////////////////////////////////////
