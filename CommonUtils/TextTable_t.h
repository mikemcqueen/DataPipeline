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

#include <exception> // temp
#include "Log.h"

#ifndef Include_TEXTTABLE_T_H
#define Include_TEXTTABLE_T_H

/////////////////////////////////////////////////////////////////////////////

class TextTable_i
{

public:

    virtual int GetRowCount() const = 0;
    virtual int GetColumnCount() const = 0;
    virtual void ClearRow(int row) = 0;
    virtual void SetText(int row, int column, const std::string& str) = 0;

    virtual wchar_t* GetRow(size_t) { throw std::runtime_error("not implemented");  };
    virtual size_t           GetRowWidth() const { throw std::runtime_error("not implemented"); };
    virtual const size_t* GetColumnWidths() const { throw std::runtime_error("not implemented"); };
    virtual void             SetEndRow(size_t ) { throw std::runtime_error("not implemented"); };

    /*
    size_t
    GetColumnWidth(
        size_t column) const
    {
       if (column >= GetColumnCount())
            throw std::invalid_argument("TextTable_i::GetColumnWidth()");
        return GetColumnWidths()[column];
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
    */
};

/////////////////////////////////////////////////////////////////////////////

template<
    int RowCount,
    int CharsPerRow,
    size_t MaxColumnCount>
class TextTableData_t
{
private:

    using type = TextTableData_t<RowCount, CharsPerRow, MaxColumnCount>;

public:

    using Row_t = wchar_t[CharsPerRow];
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
        SecureZeroMemory(this, sizeof(type));
    }

    TextTableData_t(
        const size_t* pColumnWidths,
              size_t  ColumnCount)
        :
        m_ColumnCount(ColumnCount)
    {
        SetColumnWidths(pColumnWidths, ColumnCount);
    }

    TextTableData_t(const type& rhs)
    {
        *this = rhs;
    }

    auto& operator=(const type& rhs)
    {
        if (this != &rhs) {
            memcpy(this, &rhs, sizeof(type));
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

    int GetRowCount() const    { return RowCount; }
    size_t GetCharsPerRow() const { return CharsPerRow; }

#if 1
    const wchar_t*
    GetRow(
        size_t Row) const
    {
        return const_cast<type*>(this)->GetBuffer(Row);
    }
#else
    const Row_t&
    GetRow(
        size_t Row) const
    {
        return reinterpret_cast<Row_t&>(*const_cast<type*>(this)->GetBuffer(Row));
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
        const type& text,
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
class TextTable1_t :
    public TextTable_i
{
public:

    using type = TextTable1_t<RowCount, CharsPerRow, ColumnCount>;
    using Data_t = TextTableData_t<RowCount, CharsPerRow, ColumnCount>;

private:

    Data_t m_Data;

public:

    TextTable1_t(
        const size_t* pColumnWidths,
        size_t        maxColumns)
        :
        m_Data(pColumnWidths, maxColumns)
    { }

    type& operator=(const Data_t& data)
    {
        m_Data = data;
		return *this;
    }

    //
    // TextTable_i virtual:
    //

    void ClearRow(int ) override { throw std::runtime_error("not implemented"); }
    void SetText(int , int , const std::string& )override { throw std::runtime_error("not implemented"); }

    int           GetRowCount() const override { return m_Data.GetRowCount(); }
    wchar_t*         GetRow(size_t Row) override { return m_Data.GetBuffer(Row); }
    size_t           GetRowWidth() const override { return m_Data.GetCharsPerRow(); }
    int           GetColumnCount() const override { return ColumnCount; }
    const size_t*    GetColumnWidths() const override { return m_Data.GetColumnWidths(); }
    void             SetEndRow(size_t row) override
    {
        m_Data.SetEndRow(row);
#if 1
        if (row < RowCount)
            MarkRow(row, L"END");
        for (++row; row < RowCount; ++row)
            MarkRow(row, L"UNUSED");
#endif
    }

    const wchar_t*
    GetRow(
        size_t row) const
    {
        return m_Data.GetRow(row);
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
        size_t column) const
    {
        return m_Data.GetColumnOffset(column);
    }

    bool
    CompareRow(
        size_t        iMyRow,
        const type& text,
        size_t        iTheRow) const
    {
        return m_Data.CompareRow(iMyRow, text.GetData(), iTheRow);
    }

    void
    SetColumnWidths(
        const size_t* pColumnWidths,
        size_t        columnCount)
    {
        m_Data.SetColumnWidths(pColumnWidths, columnCount);
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
        size_t row,
        const wchar_t* pszTag = nullptr) const
    {
        m_Data.DumpRow(row, pszTag);
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
        size_t column) const
    {
        return m_Data.GetColumnWidth(column);
    }

    bool
    IsEndRow(
        size_t row) const
    {
        return IsMarkedRow(row, L"END");
    }

    bool
    IsBadRow(
        size_t row) const
    {
        // TODO: hacko until TextTable_t becomes DCR::TextTable_t,
        // and DCR::ReadTable() takes it as a parameter.
        const wchar_t* pszRow = GetRow(row);
        return 0 == wcscmp(pszRow, L"BAD");
    }

protected:

    bool
    IsMarkedRow(
        size_t         row,
        const wchar_t* pszTag = nullptr) const
    {
        return m_Data.IsMarkedRow(row, pszTag);
    }

    void
    MarkRow(
        size_t row,
        wchar_t* pszTag)
    {
//Data.MarkRow(Row, pszTag);
        wchar_t* pszRow = (wchar_t*)m_Data.GetRow(row);
        pszRow[0] = Data_t::Marker;
        wcscpy_s(&pszRow[1], CharsPerRow - 1, pszTag);
    }

private:

    // Explicitly disabled:
    TextTable1_t();
    TextTable1_t(const TextTable1_t&);
    TextTable1_t& operator=(const TextTable1_t&);
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

    using type = TextTable2<Data_t>;

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
    type&
    operator=(const Data_t& Data)
    {
        m_Data = Data;
    }
*/

    //
    // TextTable_i virtual:
    //

    void ClearRow(int) override { throw std::runtime_error("not implemented"); }
    void SetText(int, int, const std::string&)override { throw std::runtime_error("not implemented"); }

    int         GetRowCount() const override { return m_Data.GetRowCount(); }
    wchar_t*       GetRow(size_t Row) override { return m_Data.GetBuffer(Row); }
    size_t         GetRowWidth() const override { return m_Data.GetCharsPerRow(); }
    int         GetColumnCount() const override { return m_Data.GetColumnCount(); }
    const size_t*  GetColumnWidths() const override { return m_Data.GetColumnWidths(); }
    void           SetEndRow(size_t row) override
    {
        m_Data.SetEndRow(row);
#if 1
        if ((int)row < GetRowCount())
            MarkRow(row, L"END");
        for (++row; (int)row < GetRowCount(); ++row)
            MarkRow(row, L"UNUSED");
#endif
    }

    const wchar_t*
    GetRow(
        size_t row) const
    {
        return m_Data.GetRow(row);
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
        const type& text,
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
        bool bAlways = false) const
    {
        m_Data.Dump(pszText, bAlways);
    }

    void
    DumpRow(
        size_t row,
        const wchar_t* pszTag = nullptr) const
    {
        m_Data.DumpRow(row, pszTag);
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
        size_t row) const
    {
        return IsMarkedRow(row, L"END");
    }

    bool
    IsBadRow(
        size_t row) const
    {
        // TODO: hacko until TextTable_t becomes DCR::TextTable_t,
        // and DCR::ReadTable() takes it as a parameter.
        const wchar_t* pszRow = GetRow(row);
        return 0 == wcscmp(pszRow, L"BAD");
    }

protected:

    bool
    IsMarkedRow(
        size_t         row,
        const wchar_t* pszTag = nullptr) const
    {
        return m_Data.IsMarkedRow(row, pszTag);
    }

    void
    MarkRow(
        size_t row,
        const wchar_t* pszTag)
    {
//Data.MarkRow(Row, pszTag);
        wchar_t* pszRow = (wchar_t*)m_Data.GetRow(row);
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

template<
    int RowCount,
    int CharsPerRow,
    int ColumnCount>
class NewTextTableData_t
{
    //    using type = NewTextTableData_t<RowCount, CharsPerRow, ColumnCount>;
public:

    /////////////////////////////////////////////////////////////////////////////
    // Column_t
    struct Column_t final
    {
        std::span<char> view;

        //auto len = std::min<int>(text.size(), str.length());
        void SetText(const std::string& str)
        {
            memcpy_s(view.data(), view.size(), str.c_str(), std::min<size_t>(view.size(), str.size()));
        }
        const std::string GetText() const { return std::string{ view.data(), view.size() }; }
    };

    /////////////////////////////////////////////////////////////////////////////
    // Row_t
    template<int CharsPerRow, int ColumnCount>
    struct RowData_t final
    {
    protected:
        friend class NewTextTableData_t;

        void
            init(std::span<const int> charsPerColumn) {
            auto row = span{ text };
            auto start = 0;
            for (size_t i = 0; i < charsPerColumn.size(); ++i) {
                auto len = charsPerColumn[i];
                columns[i].view = row.subspan(start, len);
                start += len;
            }
        }

    public:

        void Clear() { SecureZeroMemory(text.data(), text.size()); }

        std::array<char, CharsPerRow>     text;
        std::array<Column_t, ColumnCount> columns;
    };
    using Row_t = RowData_t<CharsPerRow, ColumnCount>;
 
    /////////////////////////////////////////////////////////////////////////////
    // NewTextTableData_t

    NewTextTableData_t(std::span<const int> charsPerColumn)
    {
        for (auto& row : rows_) {
            row.init(charsPerColumn);
        }
    }

    int GetRowCount() const { return RowCount; }
    int GetColumnCount() const { return ColumnCount; }
    Row_t& GetRow(int row) /*const*/ { return rows_[row]; }
    const Row_t& GetRow(int row) const { return rows_[row]; }

    void fill() {
        stringstream ss;
        for (auto i = 0; i < 10; ++i) {
            for (auto j = 0; j < 10; ++j) {
                ss << std::format("{}", j);
            }
        }
        auto len = std::min<int>(CharsPerRow, ss.str().length());
        memcpy_s(rows_[0].text.data(), CharsPerRow, ss.str().c_str(), len);
    }

    void show() {
        for (auto col = 0; col < ColumnCount; ++col) {
            auto& column = rows_[0].columns[col];
            //string s(column.data(), column.size());
            auto s{ column.GetText() };
            LogInfo(L"Col %d: '%S' (%d)", col, s.c_str(), s.length());
        }
    }

    void
    Dump(
        const wchar_t* header) const
    {
        LogInfo(L"------%s------", header);
        for (auto row : rows_) {
            LogInfo(L"Name: %S", row.columns[1].GetText().c_str());
        }
        LogInfo(L"-----------------------");
    }

private:
    std::array<Row_t, RowCount> rows_;
};

/////////////////////////////////////////////////////////////////////////////
//
// TextTable3_t
//
/////////////////////////////////////////////////////////////////////////////

template<class Data_t>
class TextTable3 :
    public TextTable_i
{
public:
    //using type = TextTable3<Data_t>;

    //explicit
    TextTable3(
        std::span<const int> charsPerColumn)
        :
        data_(charsPerColumn)
    { }

    TextTable3() = delete;
    TextTable3(const TextTable3&) = delete;

    //
    // TextTable_i virtual:
    //

    int GetRowCount() const override { return data_.GetRowCount(); }
    int GetColumnCount() const override { return data_.GetColumnCount(); }
    void ClearRow(int row) override { data_.GetRow(row).Clear(); }
    void SetText(int row, int column, const std::string& str) override
    {
        data_.GetRow(row).columns[column].SetText(str);
    }

    // Additional public methods:

    const Data_t& GetData() const { return data_; }

private:
    Data_t data_;
};

/////////////////////////////////////////////////////////////////////////////

#endif // Include_TEXTTABLE_T_H

/////////////////////////////////////////////////////////////////////////////
