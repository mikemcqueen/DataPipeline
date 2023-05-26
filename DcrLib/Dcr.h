/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// DCR.H
//
// Digital Character Recognition (a.k.a. ConvertBitmapToText)
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_DCR_H
#define Include_DCR_H

struct Rect_t;
class CSurface;
class TextTable_i;
class TableInfo_t;

/////////////////////////////////////////////////////////////////////////////

enum class DcrImpl {
    Legacy,
    Tesseract,

    Default = Tesseract
};

class TableInfo_t;

/////////////////////////////////////////////////////////////////////////////

class DCR
{
private:
  struct erased {
    struct interface_t {
      virtual ~interface_t() noexcept {}

      virtual std::string GetText(
        const CSurface* pSurface,
        const Rect_t& rect) const = 0;

      virtual int GetTableText(
        const CSurface* pSurface,
        const Rect_t& rcTable,
        const TableInfo_t& tableInfo,
        const std::vector<Rect_t>& columnRects,
        const std::vector<std::unique_ptr<CSurface>>& columnSurfaces,
        TextTable_i* pText) const = 0;
    };

    template <typename T>
    struct impl_t : interface_t {
    private:
      std::unique_ptr<T> impl_;

    public:
      impl_t(std::unique_ptr<T> impl) : impl_(std::move(impl)) {}

      /*
      constexpr impl_t(impl_t&&) noexcept = default;
      constexpr impl_t& operator=(impl_t&&) noexcept = default;
      */
      ~impl_t() noexcept override = default;

      std::string GetText(
        const CSurface* pSurface,
        const Rect_t& rect) const override
      {
        return impl_->GetText(pSurface, rect);
      }

      int GetTableText(
        const CSurface* pSurface,
        const Rect_t& rcTable,
        const TableInfo_t& tableInfo,
        const std::vector<Rect_t>& columnRects,
        const std::vector<std::unique_ptr<CSurface>>& columnSurfaces,
        TextTable_i* pText) const override
      {
        return impl_->GetTableText(pSurface, rcTable, tableInfo, columnRects, columnSurfaces, pText);
      }
    };
  };
  using impl_map_t = std::unordered_map<DcrImpl, std::unique_ptr<erased::interface_t>>;
  static impl_map_t impl_map_;

public:
  DCR(int id, std::optional<DcrImpl> method);
  DCR(const DCR&) = delete;
  DCR(const DCR&&) = delete;
  virtual ~DCR();

  // DCR virtual:
  virtual bool Initialize() { return true; }
  virtual bool TranslateSurface(CSurface*, const Rect_t&) { return true; }

  const auto id() {
    return id_;
  }

  const erased::interface_t& impl() {
    return DCR::impl(method_);
  }

  static void WriteBadBmp(
    const CSurface* pSurface,
    const RECT& rc,
    const wchar_t* pszText);

  template<typename T>
  static void add_impl(DcrImpl method, std::unique_ptr<T> impl) {
    if (impl_map_.contains(method)) {
      throw std::invalid_argument(std::format("DcrImpl added twice, method: {}", int(method)));
    }
    impl_map_.emplace(method, make_unique<erased::impl_t<T>>(std::move(impl)));
  }

  static std::unique_ptr<erased::interface_t> remove_impl(DcrImpl method) {
    if (!impl_map_.contains(method)) {
      throw std::invalid_argument(std::format("DcrImpl doesn't exist, method: {}", int(method)));
    }
    return std::move(impl_map_.extract(method).mapped());
  }

private:
  static const erased::interface_t& impl(DcrImpl method) {
    if (!impl_map_.contains(method)) {
      throw std::logic_error(std::format("DcrImpl doesn't exist, method: {}", int(method)));
    }
    return *impl_map_.at(method).get();
  }

  int id_;
  DcrImpl method_;
};

#endif // Include_DCR_H_
