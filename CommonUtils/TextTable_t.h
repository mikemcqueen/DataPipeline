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

class TextTable_i
{
public:
  virtual ~TextTable_i() = default;

  virtual int GetRowCount() const = 0;
  virtual int GetColumnCount() const = 0;
  virtual void ClearRow(int row) = 0;
  virtual void SetText(int row, int column, const std::string& str) = 0;

  virtual wchar_t* GetRow(size_t) { throw std::runtime_error("not implemented");  };
  virtual size_t GetRowWidth() const { throw std::runtime_error("not implemented"); };
  virtual const size_t* GetColumnWidths() const { throw std::runtime_error("not implemented"); };
  virtual void SetEndRow(size_t ) { throw std::runtime_error("not implemented"); };

  virtual void SetRowRect(int, const Rect_t&) {
    throw std::runtime_error("not implemented");
  }
};

template<int RowCount, int CharsPerRow, int ColumnCount>
class NewTextTableData_t {
public:
  // Column_t
  struct Column_t {
    std::span<char> view;

    //auto len = std::min<int>(text.size(), str.length());
    void SetText(const std::string& str) {
      memcpy_s(view.data(), view.size(), str.c_str(), std::min<size_t>(view.size(), str.size()));
    }
    const std::string_view GetText() const {
      return { view.data(), strnlen(view.data(), view.size()) };
    }
  };

  template<int CharsPerRow, int ColumnCount>
  struct RowData_t {
  protected:
    friend class NewTextTableData_t;

    void init(std::span<const int> charsPerColumn) {
      auto row = std::span{ text };
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
    Rect_t rect;
  };
  using Row_t = RowData_t<CharsPerRow, ColumnCount>;

  /////////////////////////////////////////////////////////////////////////////
  // NewTextTableData_t

  NewTextTableData_t(std::span<const int> charsPerColumn) {
    for (auto& row : rows_) {
      row.init(charsPerColumn);
    }
  }

  int GetRowCount() const { return RowCount; }
  int GetColumnCount() const { return ColumnCount; }
  Row_t& GetRow(int row) /*const*/ { return rows_[row]; }
  const Row_t& GetRow(int row) const { return rows_[row]; }

  void fill() {
    std::stringstream ss;
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

  void Dump(const wchar_t* header) const {
    LogInfo(L"------%s------", header);
    for (auto row : rows_) {
      LogInfo(L"Name: %S", row.columns[1].GetText().data());
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
class TextTable3 : public TextTable_i {
public:
    TextTable3(std::span<const int> charsPerColumn) : data_(charsPerColumn) {}
    TextTable3() = delete;
    TextTable3(const TextTable3&) = delete;

    // TextTable_i virtual:
    int GetRowCount() const override { return data_.GetRowCount(); }
    int GetColumnCount() const override { return data_.GetColumnCount(); }
    void ClearRow(int row) override { data_.GetRow(row).Clear(); }
    void SetText(int row, int column, const std::string& str) override {
        data_.GetRow(row).columns[column].SetText(str);
    }
    void SetRowRect(int row, const Rect_t& rect) override {
      data_.GetRow(row).rect = rect;
    }

    const Data_t& GetData() const { return data_; }

private:
    Data_t data_;
};

#endif // Include_TEXTTABLE_T_H
